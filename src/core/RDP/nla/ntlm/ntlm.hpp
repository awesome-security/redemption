/*
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/


#pragma once

#include "core/RDP/nla/sspi.hpp"
#include "core/RDP/nla/ntlm/ntlm_context.hpp"

// TODO: constants below are still globals,
// better to move them in the scope of functions/objects using them
namespace {
    //const char* NTLM_PACKAGE_NAME = "NTLM";
    const char Ntlm_Name[] = "NTLM";
    const char Ntlm_Comment[] = "NTLM Security Package";
    const SecPkgInfo NTLM_SecPkgInfo = {
        0x00082B37,             // fCapabilities
        1,                      // wVersion
        0x000A,                 // wRPCID
        0x00000B48,             // cbMaxToken
        Ntlm_Name,              // Name
        Ntlm_Comment            // Comment
    };
}

struct Ntlm_SecurityFunctionTable : public SecurityFunctionTable
{
private:
    Random & rand;
    TimeObj & timeobj;
    CREDENTIALS * credentials = nullptr;
    NTLMContext * context = nullptr;

public:
    bool hardcoded_tests = false;

    CREDENTIALS const * getCredentialHandle() const
    {
        return this->credentials;
    }

    NTLMContext const * getContextHandle() const
    {
        return this->context;
    }

public:
    explicit Ntlm_SecurityFunctionTable(Random & rand, TimeObj & timeobj)
        : rand(rand), timeobj(timeobj)
    {}

    ~Ntlm_SecurityFunctionTable()
    {
        delete this->context;
        delete this->credentials;
    }

    SEC_STATUS CompleteAuthToken(SecBufferDesc*) override { return SEC_E_UNSUPPORTED_FUNCTION; }


    // QUERY_SECURITY_PACKAGE_INFO QuerySecurityPackageInfo;
    SEC_STATUS QuerySecurityPackageInfo(SecPkgInfo * pPackageInfo) override {
        assert(pPackageInfo);
        *pPackageInfo = NTLM_SecPkgInfo;
        return SEC_E_OK;
    }

    // QUERY_CONTEXT_ATTRIBUTES QueryContextAttributes;
    SEC_STATUS QueryContextSizes(SecPkgContext_Sizes* ContextSizes) override {
        if (!ContextSizes) {
            return SEC_E_INSUFFICIENT_MEMORY;
        }
        ContextSizes->cbMaxToken = 2010;
        ContextSizes->cbMaxSignature = 16;
        ContextSizes->cbBlockSize = 0;
        ContextSizes->cbSecurityTrailer = 16;
        return SEC_E_OK;
    }

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    SEC_STATUS AcquireCredentialsHandle(
        const char * pszPrincipal, unsigned long fCredentialUse,
        Array * pvLogonID, SEC_WINNT_AUTH_IDENTITY * pAuthData
    ) override
    {
        (void)pszPrincipal;
        (void)pvLogonID;

        if (fCredentialUse == SECPKG_CRED_OUTBOUND) {
            this->credentials = new CREDENTIALS;

            if (pAuthData != nullptr) {
                this->credentials->identity.CopyAuthIdentity(*pAuthData);
            }

            return SEC_E_OK;
        }
        else if (fCredentialUse == SECPKG_CRED_INBOUND) {
            this->credentials = new CREDENTIALS;

            if (pAuthData) {
                this->credentials->identity.CopyAuthIdentity(*pAuthData);
            }
            else {
                this->credentials->identity.clear();
            }

            return SEC_E_OK;
        }

        return SEC_E_OK;
    }

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    SEC_STATUS InitializeSecurityContext(
        char* pszTargetName, unsigned long fContextReq,
        SecBufferDesc * pInput, unsigned long verbose,
        SecBufferDesc * pOutput
    ) override
    {
        if (verbose & 0x400) {
            LOG(LOG_INFO, "NTLM_SSPI::InitializeSecurityContext");
        }

        if (!this->context) {
            this->context = new NTLMContext(this->rand, this->timeobj);

            if (!this->context) {
                return SEC_E_INSUFFICIENT_MEMORY;
            }

            this->context->verbose = verbose;
            // this->context->init();
            this->context->server = false;
            if (fContextReq & ISC_REQ_CONFIDENTIALITY) {
                this->context->confidentiality = true;
            }

            // if (this->context->Workstation.size() < 1)
            //     this->context->ntlm_SetContextWorkstation(nullptr);
            if (!this->credentials) {
                return SEC_E_WRONG_CREDENTIAL_HANDLE;
            }
            this->context->ntlm_SetContextWorkstation(reinterpret_cast<uint8_t*>(pszTargetName));
            this->context->ntlm_SetContextServicePrincipalName(reinterpret_cast<uint8_t*>(pszTargetName));

            this->context->identity.CopyAuthIdentity(this->credentials->identity);
        }

        if ((!pInput) || (this->context->state == NTLM_STATE_AUTHENTICATE)) {
            if (!pOutput) {
                return SEC_E_INVALID_TOKEN;
            }
            if (pOutput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            PSecBuffer output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);
            if (!output_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (output_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            if (this->context->state == NTLM_STATE_INITIAL) {
                this->context->state = NTLM_STATE_NEGOTIATE;
            }
            if (this->context->state == NTLM_STATE_NEGOTIATE) {
                return this->context->write_negotiate(output_buffer);
            }
            return SEC_E_OUT_OF_SEQUENCE;
        }
        else {
            if (pInput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            PSecBuffer input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!input_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (input_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            // channel_bindings = sspi_FindSecBuffer(pInput, SECBUFFER_CHANNEL_BINDINGS);

            // if (channel_bindings) {
            //     this->context->Bindings.BindingsLength = channel_bindings->cbBuffer;
            //     this->context->Bindings.Bindings = (SEC_CHANNEL_BINDINGS*) channel_bindings->pvBuffer;
            // }

            if (this->context->state == NTLM_STATE_CHALLENGE) {
                this->context->read_challenge(input_buffer);

                if (!pOutput) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (pOutput->cBuffers < 1) {
                    return SEC_E_INVALID_TOKEN;
                }
                PSecBuffer output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);

                if (!output_buffer) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (output_buffer->Buffer.size() < 1) {
                    return SEC_E_INSUFFICIENT_MEMORY;
                }
                if (this->context->state == NTLM_STATE_AUTHENTICATE) {
                    return this->context->write_authenticate(output_buffer);
                }
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    SEC_STATUS AcceptSecurityContext(
        SecBufferDesc * pInput, unsigned long fContextReq,
        SecBufferDesc * pOutput
    ) override {
        if (!this->context) {
            this->context = new(std::nothrow) NTLMContext(this->rand, this->timeobj);

            if (!this->context) {
                return SEC_E_INSUFFICIENT_MEMORY;
            }

            this->context->server = true;
            if (fContextReq & ASC_REQ_CONFIDENTIALITY) {
                this->context->confidentiality = true;
            }
            if (!this->credentials) {
                return SEC_E_WRONG_CREDENTIAL_HANDLE;
            }
            this->context->identity.CopyAuthIdentity(this->credentials->identity);

            this->context->ntlm_SetContextServicePrincipalName(nullptr);
        }

        if (this->context->state == NTLM_STATE_INITIAL) {
            this->context->state = NTLM_STATE_NEGOTIATE;

            if (!pInput) {
                return SEC_E_INVALID_TOKEN;
            }
            if (pInput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            PSecBuffer input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!input_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (input_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            /*SEC_STATUS status = */this->context->read_negotiate(input_buffer);

            if (this->context->state == NTLM_STATE_CHALLENGE) {
                if (!pOutput) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (pOutput->cBuffers < 1) {
                    return SEC_E_INVALID_TOKEN;
                }
                PSecBuffer output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);

                if (!output_buffer->BufferType) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (output_buffer->Buffer.size() < 1) {
                    return SEC_E_INSUFFICIENT_MEMORY;
                }
                return this->context->write_challenge(output_buffer);
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }
        else if (this->context->state == NTLM_STATE_AUTHENTICATE) {
            if (!pInput) {
                return SEC_E_INVALID_TOKEN;
            }
            if (pInput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            PSecBuffer input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!input_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (input_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }

            if (this->hardcoded_tests) {
                this->context->identity.SetPasswordFromUtf8(reinterpret_cast<uint8_t const *>("Pénélope"));
            }
            SEC_STATUS status = this->context->read_authenticate(input_buffer);

            if (pOutput) {
                size_t i;
                for (i = 0; i < pOutput->cBuffers; i++) {
                    pOutput->pBuffers[i].Buffer.init(0);
                    pOutput->pBuffers[i].BufferType = SECBUFFER_TOKEN;
                }
            }
            return status;
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    SEC_STATUS EncryptMessage(PSecBufferDesc pMessage, unsigned long MessageSeqNo) override {
        int length;
        uint8_t* data;
        uint32_t SeqNo(MessageSeqNo);
        uint8_t checksum[8];
        uint8_t* signature;
        uint32_t version = 1;
        PSecBuffer data_buffer = nullptr;
        PSecBuffer signature_buffer = nullptr;
        if (!this->context) {
            return SEC_E_NO_CONTEXT;
        }
        if (this->context->verbose & 0x400) {
            LOG(LOG_INFO, "NTLM_SSPI::EncryptMessage");
        }
        for (unsigned long index = 0; index < pMessage->cBuffers; index++) {
            if (pMessage->pBuffers[index].BufferType == SECBUFFER_DATA) {
                data_buffer = &pMessage->pBuffers[index];
            }
            else if (pMessage->pBuffers[index].BufferType == SECBUFFER_TOKEN) {
                signature_buffer = &pMessage->pBuffers[index];
            }
        }

        if (!data_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        if (!signature_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        /* Copy original data buffer */
        length = data_buffer->Buffer.size();
        data = new uint8_t[length];
        memcpy(data, data_buffer->Buffer.get_data(), length);

        /* Compute the HMAC-MD5 hash of ConcatenationOf(seq_num,data) using the client signing key */
        uint8_t digest[SslMd5::DIGEST_LENGTH];
        SslHMAC_Md5 hmac_md5(this->context->SendSigningKey, 16);
        hmac_md5.update(reinterpret_cast<uint8_t*>(&SeqNo), 4);
        hmac_md5.update(data, length);
        hmac_md5.final(digest);

        /* Encrypt message using with RC4, result overwrites original buffer */

        if (this->context->confidentiality) {
            this->context->SendRc4Seal.crypt(length, data, data_buffer->Buffer.get_data());
        }
        else {
            data_buffer->Buffer.copy(data, length);
        }


// #ifdef WITH_DEBUG_NTLM
        // LOG(LOG_ERR, "======== ENCRYPT ==========");
        // LOG(LOG_ERR, "signing key (length = %d)\n", 16);
        // hexdump_c(this->context->SendSigningKey, 16);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Digest (length = %d)\n", sizeof(digest));
        // hexdump_c(digest, sizeof(digest));
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Data Buffer (length = %d)\n", length);
        // hexdump_c(data, length);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Encrypted Data Buffer (length = %d)\n", data_buffer->Buffer.size());
        // hexdump_c(data_buffer->Buffer.get_data(), data_buffer->Buffer.size());
        // LOG(LOG_ERR, "\n");
// #endif

        delete [] data;

        /* RC4-encrypt first 8 bytes of digest */
        this->context->SendRc4Seal.crypt(8, digest, checksum);

        signature = signature_buffer->Buffer.get_data();

        /* Concatenate version, ciphertext and sequence number to build signature */
        memcpy(signature, &version, 4);
        memcpy(&signature[4], checksum, 8);
        memcpy(&signature[12], &SeqNo, 4);
        this->context->SendSeqNum++;

// #ifdef WITH_DEBUG_NTLM
        // LOG(LOG_ERR, "Signature (length = %d)\n", signature_buffer->Buffer.size());
        // hexdump_c(signature_buffer->Buffer.get_data(), signature_buffer->Buffer.size());
        // LOG(LOG_ERR, "\n");
// #endif

        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    SEC_STATUS DecryptMessage(PSecBufferDesc pMessage, unsigned long MessageSeqNo) override {
        int length = 0;
        uint8_t* data = nullptr;
        uint32_t SeqNo(MessageSeqNo);
        uint8_t digest[SslMd5::DIGEST_LENGTH] = {};
        uint8_t checksum[8] = {};
        uint32_t version = 1;
        uint8_t expected_signature[16] = {};
        PSecBuffer data_buffer = nullptr;
        PSecBuffer signature_buffer = nullptr;
        if (!this->context) {
            return SEC_E_NO_CONTEXT;
        }
        if (this->context->verbose & 0x400) {
            LOG(LOG_INFO, "NTLM_SSPI::DecryptMessage");
        }

        for (unsigned long index = 0; index < pMessage->cBuffers; index++) {
            if (pMessage->pBuffers[index].BufferType == SECBUFFER_DATA) {
                data_buffer = &pMessage->pBuffers[index];
            }
            else if (pMessage->pBuffers[index].BufferType == SECBUFFER_TOKEN) {
                signature_buffer = &pMessage->pBuffers[index];
            }
        }

        if (!data_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        if (!signature_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        /* Copy original data buffer */
        length = data_buffer->Buffer.size();
        data = new uint8_t[length];
        memcpy(data, data_buffer->Buffer.get_data(), length);

        /* Decrypt message using with RC4, result overwrites original buffer */

        if (this->context->confidentiality) {
            this->context->RecvRc4Seal.crypt(length, data, data_buffer->Buffer.get_data());
        }
        else {
            data_buffer->Buffer.copy(data, length);
        }

        /* Compute the HMAC-MD5 hash of ConcatenationOf(seq_num,data) using the client signing key */
        SslHMAC_Md5 hmac_md5(this->context->RecvSigningKey, 16);
        hmac_md5.update(reinterpret_cast<uint8_t*>(&SeqNo), 4);
        hmac_md5.update(data_buffer->Buffer.get_data(), data_buffer->Buffer.size());
        hmac_md5.final(digest);

// #ifdef WITH_DEBUG_NTLM
        // LOG(LOG_ERR, "======== DECRYPT ==========");
        // LOG(LOG_ERR, "signing key (length = %d)\n", 16);
        // hexdump_c(this->context->RecvSigningKey, 16);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Digest (length = %d)\n", sizeof(digest));
        // hexdump_c(digest, sizeof(digest));
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Encrypted Data Buffer (length = %d)\n", length);
        // hexdump_c(data, length);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Data Buffer (length = %d)\n", data_buffer->Buffer.size());
        // hexdump_c(data_buffer->Buffer.get_data(), data_buffer->Buffer.size());
        // LOG(LOG_ERR, "\n");
// #endif

        delete [] data;

        /* RC4-encrypt first 8 bytes of digest */
        this->context->RecvRc4Seal.crypt(8, digest, checksum);

        /* Concatenate version, ciphertext and sequence number to build signature */
        memcpy(expected_signature, &version, 4);
        memcpy(&expected_signature[4], checksum, 8);
        memcpy(&expected_signature[12], &SeqNo, 4);
        this->context->RecvSeqNum++;

        if (memcmp(signature_buffer->Buffer.get_data(), expected_signature, 16) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
            LOG(LOG_ERR, "Expected Signature:");
            hexdump_c(expected_signature, 16);
            LOG(LOG_ERR, "Actual Signature:");
            hexdump_c(signature_buffer->Buffer.get_data(), 16);

            return SEC_E_MESSAGE_ALTERED;
        }

        return SEC_E_OK;
    }

    // IMPERSONATE_SECURITY_CONTEXT ImpersonateSecurityContext;
    SEC_STATUS ImpersonateSecurityContext() override {
        return SEC_E_OK;
    }
    // REVERT_SECURITY_CONTEXT RevertSecurityContext;
    SEC_STATUS RevertSecurityContext() override {
        return SEC_E_OK;
    }
};


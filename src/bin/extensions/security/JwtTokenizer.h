#ifndef REDISTEST_JWTTOKENIZER_H
#define REDISTEST_JWTTOKENIZER_H

#include <string>
#include <jwt-cpp/jwt.h>

class JwtTokenizer {
public:
    typedef jwt::builder<jwt::traits::kazuho_picojson> builder_t;

    class VerifyBuilder {
        std::string secret_key_;
        std::string issuer_;
        JwtTokenizer *outer;
    public:

        explicit VerifyBuilder(JwtTokenizer *o) : outer(o) {

        }

        VerifyBuilder secret_key(const std::string &secret_key) {
            secret_key_ = secret_key;
            return *this;
        }

        VerifyBuilder
        issuer(const std::string &issuer) {
            issuer_ = issuer;
            return *this;
        }

        bool
        verify(const std::string &token) {
            auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{secret_key_})
                    .with_issuer(issuer_);
            try {
                verifier.verify(outer->decode(token));
                return true;
            } catch (std::exception &e) {
                return false;
            };
        }
    };

    JwtTokenizer() : verifyBuilder_(nullptr) {};

    std::string generate();

    auto create() {
        return token_builder_ = jwt::create();
    }

    void
    set_secret_key(const std::string &key) {
        secret_key_ = key;
    };

    void
    set_issuer(const std::string &issuer) {
        issuer_ = issuer;
    };

    jwt::decoded_jwt<jwt::traits::kazuho_picojson>
    decode(const std::string &token) {
        return jwt::decode(token);
    };

    VerifyBuilder
    verifier();

    builder_t set_payload_claim(const std::string &key, const std::string &value);

    ~JwtTokenizer() {
        delete verifyBuilder_;
        verifyBuilder_ = nullptr;
    };

private:
    std::string issuer_;
    std::string secret_key_;
    builder_t token_builder_;
    VerifyBuilder *verifyBuilder_;
};

#endif //REDISTEST_JWTTOKENIZER_H

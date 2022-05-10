#include "JwtTokenizer.h"

JwtTokenizer::VerifyBuilder
JwtTokenizer::verifier() {
    delete verifyBuilder_;
    verifyBuilder_ = new VerifyBuilder(this);
    return *verifyBuilder_;
}


JwtTokenizer::builder_t
JwtTokenizer::set_payload_claim(const std::string &key, const std::string &value) {
    return token_builder_.set_payload_claim(key, jwt::claim(value));
}

std::string JwtTokenizer::generate() {
    return token_builder_
            .set_issuer(issuer_)
            .set_type("JWS")
            .sign(jwt::algorithm::hs256{secret_key_});
}
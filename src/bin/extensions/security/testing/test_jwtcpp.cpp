
#include <jwt-cpp/jwt.h>
#include "iostream"
#include "../JwtTokenizer.h"

using namespace std;
std::string token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJpc3MiOiJhdXRoMCIsInNhbXBsZSI6InRlc3QifQ.lQm3N2bVlqt2-1L-FsOjtR6uE-L4E9zJutMWKIe1v1M";

void
test_jwt() {
    auto decoded = jwt::decode(token);
    //Get all payload claims
    //Get all header claims
    for (auto &e2: decoded.get_header_claims())
        std::cout << e2.first << " = " << e2.second.to_json() << std::endl;
    cout << "---------------------" << endl;

    for (auto &e1: decoded.get_payload_claims()) {
        std::cout << e1.first << " = " << e1.second.to_json() << std::endl;
    }
    cout << decoded.get_payload_claim("sample");
    //Create a verifier using the default clock then return verifier instance
    /* allow_algorithm()
          Add an algorithm available for checking.
          param alg Algorithm to allow
          return *this to allow chaining*/
    auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{"secret"}).with_issuer("auth0");
    verifier.verify(decoded);
    auto token_1 = jwt::create()
            .set_issuer("auth0")
            .set_type("JWS")
            .set_payload_claim("sample", jwt::claim(std::string("test")))
            .sign(jwt::algorithm::hs256{"secret"});
    std::cout << "token_1=" << token_1 << endl;
}

void test_tokenizer() {
    JwtTokenizer jwtTokenizer;
    cout << jwtTokenizer.verifier().secret_key("secret").issuer("auth0").verify(token);
}

int main(int argc, const char **argv) {
    test_jwt();
}
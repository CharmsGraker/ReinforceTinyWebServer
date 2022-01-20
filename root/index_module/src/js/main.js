function setFormMessage(formElement, type, message) {
    const messageElement = formElement.querySelector(".form__message");
    messageElement.textContent = message;
    messageElement.classList.remove("form__message--success", "form__message--error");
    messageElement.classList.add(`form__message--${type}`);
}

function setInputError(inputElement, message) {
    inputElement.classList.add("form__input--error");
    inputElement.parentElement.querySelector(".form__input-error-message").textContent = message;

}

function clearInputError(inputElement) {
    inputElement.classList.remove("form__input--error")
    inputElement.parentElement.querySelector(".form__input-error-message").textContent = "";

}

document.addEventListener("DOMContentLoaded", () => {
    const loginForm = document.querySelector("#login");
    // const createAccountForm = document.querySelector("#createAccount");
    //
    // document.querySelector("#linkCreateAccount").addEventListener("click", e => {
    //     e.preventDefault();
    //     loginForm.classList.add("form-hidden");
    //     createAccountForm.classList.remove("form-hidden");
    // });

    let userNameOK = true;
    const username = document.getElementById('user');
    const password = document.getElementById('password');
    const usernameValue = username.value.trim();
    const passwordValue = password.value.trim();




        // inputElement.addEventListener("input", e => {
        //     clearInputError(inputElement);
        // });
    let usernameValid = true;
    let passwordValid = true;
    // // setFormMessage(loginForm, "error", "Invalid username/password combination");
    // document.querySelectorAll(".form__input").forEach(inputElement => {
    //     inputElement.addEventListener("blur", e => {
    //         if (e.target.id === "user" && e.target.value.length >= 0 && e.target.value.length < 3) {
    //             setInputError(inputElement, "Username must be at least 3 character.");
    //             userNameOK = false;
    //         }
    //
    //     });
    //
    //     // inputElement.addEventListener("input", e => {
    //     //     clearInputError(inputElement);
    //     // });
    // });
    document.querySelectorAll(".form__input").forEach(inputElement => {
        inputElement.addEventListener("blur", e => {
            if (e.target.id === "user" && e.target.value.length >= 0 && e.target.value.length < 3) {
                setInputError(inputElement, "Username must be at least 3 character.");
                usernameValid = false;
            } else {
                inputElement.addEventListener("input", e => {
                    clearInputError(inputElement);
                });
                usernameValid = true;
            }

            if (e.target.id === "password" && e.target.value.length <= 3) {
                setInputError(inputElement, "Please input valid password.");
                passwordValid = false;
            } else {
                inputElement.addEventListener("input", e => {
                    clearInputError(inputElement);
                });
                passwordValid = true;
            }
        });
    });

    loginForm.addEventListener("submit", e => {
        e.preventDefault();


        console.log(passwordValid);
        if (usernameValid && passwordValid) {
            loginForm.submit();
        }

    });

});
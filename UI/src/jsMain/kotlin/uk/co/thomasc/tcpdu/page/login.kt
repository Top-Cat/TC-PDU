package uk.co.thomasc.tcpdu.page

import external.Axios
import external.generateConfig
import kotlinx.browser.document
import kotlinx.serialization.Serializable
import react.Props
import react.dom.FormAction
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.form
import react.dom.html.ReactHTML.i
import react.dom.html.ReactHTML.input
import react.router.useNavigate
import react.useRef
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.util.NewNavOption
import web.autofill.AutoFillNormalField
import web.cssom.ClassName
import web.form.FormMethod
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

@Serializable
data class LoginRequest(val user: String, val pass: String)

val loginPage = fcmemo<Props>("Login") {
    val history = useNavigate()

    val userRef = useRef<HTMLInputElement>()
    val passRef = useRef<HTMLInputElement>()

    div {
        className = ClassName("login-form card border-dark")
        div {
            className = ClassName("card-header")
            +"Sign in"
        }
        form {
            className = ClassName("card-body")
            method = FormMethod.post
            action = "/login".unsafeCast<FormAction>()
            onSubmit = { ev ->
                ev.preventDefault()
                val req = LoginRequest(userRef.current?.value ?: "", passRef.current?.value ?: "")

                Axios.post<String>("$apiRoot/login", req, generateConfig<LoginRequest, String>()).then {
                    // Assume success
                    history.invoke("/", NewNavOption)

                    // TODO: Validate this cookie elsewhere
                    // console.log(document.cookie)
                }.catch {
                    if (it.asDynamic().response?.status == 401) {
                        console.log("Bad credentials")
                    } else {
                        console.log(it)
                        console.log("Error during login")
                    }
                }
            }

            // TODO: Show errors
            input {
                type = InputType.text
                className = ClassName("form-control")
                key = "username"
                name = "username"
                placeholder = "Username"
                required = true
                autoFocus = true
                autoComplete = AutoFillNormalField.username
                ref = userRef
            }
            input {
                type = InputType.password
                className = ClassName("form-control")
                key = "password"
                name = "password"
                placeholder = "Password"
                required = true
                autoComplete = AutoFillNormalField.currentPassword
                ref = passRef
            }
            div {
                className = ClassName("d-grid")
                button {
                    className = ClassName("btn btn-success")
                    type = ButtonType.submit
                    i { className = ClassName("fas fa-sign-in-alt") }
                    +" Sign in"
                }
            }
        }
    }
}

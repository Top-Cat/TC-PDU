package uk.co.thomasc.tcpdu.page

import external.Axios
import external.generateConfig
import kotlinx.browser.document
import kotlinx.html.ButtonType
import kotlinx.html.FormMethod
import kotlinx.html.InputType
import kotlinx.html.js.onSubmitFunction
import kotlinx.serialization.Serializable
import org.w3c.dom.HTMLInputElement
import react.Props
import react.dom.button
import react.dom.div
import react.dom.form
import react.dom.i
import react.dom.input
import react.fc
import react.router.useNavigate
import react.useRef
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.util.NewNavOption

@Serializable
data class LoginRequest(val user: String, val pass: String)

val loginPage = fc<Props> {
    val history = useNavigate()

    val userRef = useRef<HTMLInputElement>()
    val passRef = useRef<HTMLInputElement>()

    div("login-form card border-dark") {
        div("card-header") {
            +"Sign in"
        }
        form(classes = "card-body", method = FormMethod.post, action = "/login") {
            attrs.onSubmitFunction = { ev ->
                ev.preventDefault()
                val req = LoginRequest(userRef.current?.value ?: "", passRef.current?.value ?: "")

                Axios.post<String>("$apiRoot/login", req, generateConfig<LoginRequest, String>()).then {
                    // Assume success
                    history.invoke("/", NewNavOption)

                    // TODO: Validate this cookie elsewhere
                    console.log(document.cookie)
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
            input(type = InputType.text, classes = "form-control") {
                key = "username"
                attrs.name = "username"
                attrs.placeholder = "Username"
                attrs.required = true
                attrs.autoFocus = true
                attrs.attributes["autocomplete"] = "username"
                ref = userRef
            }
            input(type = InputType.password, classes = "form-control") {
                key = "password"
                attrs.name = "password"
                attrs.placeholder = "Password"
                attrs.required = true
                attrs.attributes["autocomplete"] = "current-password"
                ref = passRef
            }
            div("d-grid") {
                button(classes = "btn btn-success", type = ButtonType.submit) {
                    i("fas fa-sign-in-alt") {}
                    +" Sign in"
                }
            }
        }
    }
}

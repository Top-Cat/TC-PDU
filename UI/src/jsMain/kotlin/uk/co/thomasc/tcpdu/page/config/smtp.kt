package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import kotlinx.html.ButtonType
import kotlinx.html.InputType
import kotlinx.html.id
import kotlinx.html.js.onClickFunction
import kotlinx.html.role
import org.w3c.dom.HTMLInputElement
import react.dom.button
import react.dom.defaultValue
import react.dom.div
import react.dom.form
import react.dom.i
import react.dom.input
import react.dom.label
import react.dom.span
import react.fc
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.page.LogConfig
import uk.co.thomasc.tcpdu.page.SmtpConfig
import uk.co.thomasc.tcpdu.page.handleForbidden

val smtpConfig = fc<ConfigProps> { props ->
    val history = useNavigate()

    val hostRef = useRef<HTMLInputElement>()
    val portRef = useRef<HTMLInputElement>()
    val userRef = useRef<HTMLInputElement>()
    val passRef = useRef<HTMLInputElement>()
    val toRef = useRef<HTMLInputElement>()
    val fromRef = useRef<HTMLInputElement>()

    val (showSmtpPassword, setShowSmtpPassword) = useState(false)

    props.config?.let { config ->
        div("card border-primary") {
            div("card-header") {
                +"SMTP"
            }
            div("card-body") {
                form {
                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "smtp-host"
                            +"Host"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "smtp.example.com"
                            attrs.id = "smtp-host"
                            attrs.defaultValue = config.log.smtp?.host ?: ""
                            ref = hostRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "smtp-port"
                            +"Port"
                        }
                        input(InputType.number, classes = "form-control w-25") {
                            attrs.placeholder = "587"
                            attrs.id = "smtp-port"
                            attrs.defaultValue = config.log.smtp?.port?.toString() ?: ""
                            ref = portRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "smtp-user"
                            +"User"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "smtp-user"
                            attrs.id = "smtp-user"
                            attrs.defaultValue = config.log.smtp?.user ?: ""
                            ref = userRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "smtp-pw"
                            +"Password"
                        }
                        div("input-group") {
                            input(if (showSmtpPassword) InputType.text else InputType.password, classes = "form-control") {
                                attrs.placeholder = "********"
                                attrs.id = "smtp-pw"
                                attrs.defaultValue = config.log.smtp?.password ?: ""
                                ref = passRef
                            }
                            span("input-group-text") {
                                i("fas fa-eye" + if (showSmtpPassword) "" else "-slash") {
                                    attrs.onClickFunction = {
                                        setShowSmtpPassword(!showSmtpPassword)
                                    }
                                    attrs.role = "button"
                                }
                            }
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "smtp-from"
                            +"From"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "from@example.com"
                            attrs.id = "smtp-from"
                            attrs.defaultValue = config.log.smtp?.from ?: ""
                            ref = fromRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "smtp-to"
                            +"To"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "to@example.com"
                            attrs.id = "smtp-to"
                            attrs.defaultValue = config.log.smtp?.to ?: ""
                            ref = toRef
                        }
                    }

                    button(type = ButtonType.submit, classes = "btn btn-primary") {
                        attrs.onClickFunction = { ev ->
                            ev.preventDefault()
                            val newSmtpConfig = SmtpConfig(
                                hostRef.current?.value,
                                portRef.current?.value?.toIntOrNull(),
                                userRef.current?.value,
                                passRef.current?.value,
                                fromRef.current?.value,
                                toRef.current?.value
                            )
                            Axios.post<String>(
                                "$apiRoot/config/log",
                                LogConfig(smtp = newSmtpConfig),
                                generateConfig<LogConfig, String>()
                            ).then {
                                // TODO: Show toast
                                console.log("Success")
                            }.handleForbidden(history)
                        }
                        +"Save"
                    }
                }
            }
        }
    }
}

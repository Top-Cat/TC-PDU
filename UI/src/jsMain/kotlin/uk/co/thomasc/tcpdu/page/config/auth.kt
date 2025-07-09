package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import react.dom.aria.AriaRole
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.form
import react.dom.html.ReactHTML.i
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.dom.html.ReactHTML.span
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.AuthConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.autofill.AutoFillNormalField
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

val authConfig = fcmemo<ConfigProps>("Auth Config") { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)

    val (currentPassword, setCurrentPassword) = useState("")
    val (adminPassword, setAdminPassword) = useState("")
    val (adminPasswordRep, setAdminPasswordRep) = useState("")

    val validityPeriodRef = useRef<HTMLInputElement>()
    val (showCurrentPassword, setShowCurrentPassword) = useState(false)
    val (showAdminPassword, setShowAdminPassword) = useState(false)
    val (showAdminPasswordRep, setShowAdminPasswordRep) = useState(false)

    props.config?.let { config ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"Auth"
            }
            div {
                className = ClassName("card-body")
                if (success == true) {
                    success { +"Config saved" }
                } else if (success == false) {
                    errors { +"Unknown error" }
                }

                form {
                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "auth-period"
                            +"Session Validity Period (Seconds)"
                        }
                        input {
                            type = InputType.number
                            className = ClassName("form-control w-50")
                            key = "auth-period"
                            placeholder = "86400"
                            id = "auth-period"
                            defaultValue = config.auth.validityPeriod?.toString() ?: ""
                            ref = validityPeriodRef
                        }
                    }

                    // For autocomplete
                    input {
                        className = ClassName("d-none")
                        autoComplete = AutoFillNormalField.username
                        type = InputType.text
                        name = "username"
                        value = "admin"
                        readOnly = true
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "auth-pw-old"
                            +"Current Admin Password"
                        }
                        div {
                            className = ClassName("input-group")
                            input {
                                type = if (showCurrentPassword) InputType.text else InputType.password
                                className = ClassName("form-control")
                                autoComplete = AutoFillNormalField.currentPassword
                                key = "auth-pw-old"
                                placeholder = "********"
                                id = "auth-pw-old"
                                value = currentPassword
                                onChange = {
                                    setCurrentPassword(it.currentTarget.value)
                                }
                            }
                            span {
                                className = ClassName("input-group-text")
                                i {
                                    className = ClassName("fas fa-eye" + if (showCurrentPassword) "" else "-slash")
                                    onClick = {
                                        setShowCurrentPassword(!showCurrentPassword)
                                    }
                                    role = AriaRole.button
                                }
                            }
                        }
                    }
                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "auth-pw"
                            +"New Admin Password"
                        }
                        div {
                            className = ClassName("input-group")
                            input {
                                type = if (showAdminPassword) InputType.text else InputType.password
                                className = ClassName("form-control")
                                autoComplete = AutoFillNormalField.newPassword
                                key = "auth-pw"
                                placeholder = "********"
                                id = "auth-pw"
                                value = adminPassword
                                onChange = {
                                    setAdminPassword(it.currentTarget.value)
                                }
                            }
                            span {
                                className = ClassName("input-group-text")
                                i {
                                    className = ClassName("fas fa-eye" + if (showAdminPassword) "" else "-slash")
                                    onClick = {
                                        setShowAdminPassword(!showAdminPassword)
                                    }
                                    role = AriaRole.button
                                }
                            }
                        }
                    }
                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "auth-pw-rpt"
                            +"Repeat New Password"
                        }
                        div {
                            className = ClassName("input-group")
                            input {
                                type = if (showAdminPasswordRep) InputType.text else InputType.password
                                className = ClassName("form-control")
                                autoComplete = AutoFillNormalField.newPassword
                                key = "auth-pw-rpt"
                                placeholder = "********"
                                id = "auth-pw-rpt"
                                value = adminPasswordRep
                                onChange = {
                                    setAdminPasswordRep(it.currentTarget.value)
                                }
                            }
                            span {
                                className = ClassName("input-group-text")
                                i {
                                    className = ClassName("fas fa-eye" + if (showAdminPasswordRep) "" else "-slash")
                                    onClick = {
                                        setShowAdminPasswordRep(!showAdminPasswordRep)
                                    }
                                    role = AriaRole.button
                                }
                            }
                        }
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-primary me-2")
                        onClick = { ev ->
                            ev.preventDefault()

                            val authConfig = AuthConfig(
                                validityPeriod = validityPeriodRef.current?.value?.toIntOrNull(),
                                oldPassword = currentPassword,
                                adminPassword = if (adminPassword == adminPasswordRep) adminPassword else null
                            )
                            Axios.post<String>("$apiRoot/config/auth", authConfig, generateConfig<AuthConfig, String>())
                                .then {
                                    setSuccess(true)
                                    setAdminPassword("")
                                    setAdminPasswordRep("")
                                    setCurrentPassword("")
                                    props.updateCallback(config.copy(auth = authConfig))
                                }
                                .handleForbidden(history)
                                .catch {
                                    setSuccess(false)
                                }
                        }
                        +"Save"
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-danger")
                        onClick = { ev ->
                            ev.preventDefault()
                            val authConfig = AuthConfig(
                                updateKey = true
                            )
                            Axios.post<String>("$apiRoot/config/auth", authConfig, generateConfig<AuthConfig, String>())
                                .then {
                                    setSuccess(true)
                                    props.updateCallback(config.copy(auth = authConfig))
                                }
                                .handleForbidden(history)
                                .catch {
                                    setSuccess(false)
                                }
                        }
                        +"Reset Session Key"
                    }
                }
            }
        }
    }
}

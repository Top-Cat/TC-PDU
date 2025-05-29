package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.axiosGet
import external.generateConfig
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.form
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.dom.html.ReactHTML.option
import react.dom.html.ReactHTML.select
import react.router.useNavigate
import react.useEffectOnce
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.NtpConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.HTMLSelectElement
import web.html.InputType

val ntpConfig = fcmemo<ConfigProps>("NTP Config") { props ->
    val history = useNavigate()
    val (success, setSuccess) = useState<Boolean?>(null)
    val (timezones, setTimezones) = useState(mapOf<String, String>())
    val (selectedTimezone, setSelectedTimezone) = useState("Europe/London")

    val hostRef = useRef<HTMLInputElement>()
    val offsetRef = useRef<HTMLSelectElement>()

    useEffectOnce {
        axiosGet<Map<String, String>>("https://raw.githubusercontent.com/nayarsystems/posix_tz_db/master/zones.json").then {
            setTimezones(it)
            setSelectedTimezone(it.toList().firstOrNull { (_, v) -> v == props.config?.ntp?.tz }?.first ?: "")
        }
    }

    props.config?.let { config ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"NTP"
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
                            htmlFor = "ntp-host"
                            +"Host"
                        }
                        input {
                            type = InputType.text
                            className = ClassName("form-control")
                            placeholder = "pool.ntp.org"
                            id = "ntp-host"
                            defaultValue = config.ntp.host ?: ""
                            ref = hostRef
                        }
                    }

                    div {
                        label {
                            className = ClassName("form-label")
                            htmlFor = "ntp-offset"
                            +"Time Zone"
                        }
                        select {
                            className = ClassName("form-control")
                            id = "ntp-offset"
                            value = selectedTimezone
                            onChange = { ev ->
                                setSelectedTimezone(ev.target.value)
                            }
                            timezones
                                .forEach {
                                    option {
                                        +it.key
                                    }
                                }
                            ref = offsetRef
                        }
                    }

                    button {
                        type = ButtonType.submit
                        className = ClassName("btn btn-primary")
                        onClick = { ev ->
                            ev.preventDefault()

                            val newConfig = NtpConfig(hostRef.current?.value, timezones[offsetRef.current?.value])
                            Axios.post<String>("$apiRoot/config/ntp", newConfig, generateConfig<NtpConfig, String>())
                                .then {
                                    setSuccess(true)
                                    props.updateCallback(config.copy(ntp = newConfig))
                                }
                                .handleForbidden(history)
                                .catch {
                                    setSuccess(false)
                                }
                        }
                        +"Save"
                    }
                }
            }
        }
    }
}

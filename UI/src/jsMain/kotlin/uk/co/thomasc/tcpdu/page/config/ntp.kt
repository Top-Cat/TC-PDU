package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.axiosGet
import external.generateConfig
import kotlinx.html.ButtonType
import kotlinx.html.InputType
import kotlinx.html.id
import kotlinx.html.js.onChangeFunction
import kotlinx.html.js.onClickFunction
import org.w3c.dom.HTMLInputElement
import org.w3c.dom.HTMLSelectElement
import react.dom.button
import react.dom.defaultValue
import react.dom.div
import react.dom.form
import react.dom.input
import react.dom.label
import react.dom.option
import react.dom.select
import react.fc
import react.router.useNavigate
import react.useEffectOnce
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.errors
import uk.co.thomasc.tcpdu.page.NtpConfig
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.success

val ntpConfig = fc<ConfigProps> { props ->
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
        div("card border-primary") {
            div("card-header") {
                +"NTP"
            }
            div("card-body") {
                if (success == true) {
                    success { +"Config saved" }
                } else if (success == false) {
                    errors { +"Unknown error" }
                }

                form {
                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "ntp-host"
                            +"Host"
                        }
                        input(InputType.text, classes = "form-control") {
                            attrs.placeholder = "pool.ntp.org"
                            attrs.id = "ntp-host"
                            attrs.defaultValue = config.ntp.host ?: ""
                            ref = hostRef
                        }
                    }

                    div("form-group") {
                        label("form-label") {
                            attrs.htmlFor = "ntp-offset"
                            +"Time Zone"
                        }
                        select("form-control") {
                            attrs.id = "ntp-offset"
                            attrs.value = selectedTimezone
                            attrs.onChangeFunction = { ev ->
                                setSelectedTimezone((ev.target as HTMLSelectElement).value)
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

                    button(type = ButtonType.submit, classes = "btn btn-primary") {
                        attrs.onClickFunction = { ev ->
                            ev.preventDefault()

                            Axios.post<String>(
                                "$apiRoot/config/ntp",
                                NtpConfig(hostRef.current?.value, timezones[offsetRef.current?.value]),
                                generateConfig<NtpConfig, String>()
                            ).then { setSuccess(true) }.handleForbidden(history).catch {
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

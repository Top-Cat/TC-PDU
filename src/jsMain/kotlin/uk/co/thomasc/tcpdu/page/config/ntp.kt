package uk.co.thomasc.tcpdu.page.config

import external.Axios
import external.generateConfig
import kotlinx.datetime.TimeZone
import kotlinx.datetime.internal.JSJoda.Clock
import kotlinx.datetime.internal.JSJoda.Instant
import kotlinx.datetime.internal.JSJoda.ZoneId
import kotlinx.html.ButtonType
import kotlinx.html.InputType
import kotlinx.html.id
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
import react.useRef
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.page.NtpConfig
import uk.co.thomasc.tcpdu.page.handleForbidden

val ntpConfig = fc<ConfigProps> { props ->
    val history = useNavigate()

    val hostRef = useRef<HTMLInputElement>()
    val offsetRef = useRef<HTMLSelectElement>()

    props.config?.let { config ->
        div("card border-primary") {
            div("card-header") {
                +"NTP"
            }
            div("card-body") {
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
                            attrs.value = config.ntp.offset.toString()
                            val now = Instant.now(Clock.systemUTC())
                            TimeZone.availableZoneIds
                                .asSequence()
                                .filter { it.startsWith("Etc/GMT") }
                                .map { ZoneId.of(it) }
                                .filter { it.rules().isFixedOffset() } // PDU doesn't support DST
                                .map { it to -it.rules().offset(now).totalSeconds() }
                                .sortedBy { it.second }
                                .forEach {
                                    option {
                                        attrs.value = "${it.second}"
                                        +it.first.id()
                                    }
                                }
                            ref = offsetRef
                        }
                    }

                    button(type = ButtonType.submit, classes = "btn btn-primary") {
                        attrs.onClickFunction = { ev ->
                            ev.preventDefault()
                            val offset = offsetRef.current?.value?.toIntOrNull()

                            Axios.post<String>(
                                "$apiRoot/config/ntp",
                                NtpConfig(hostRef.current?.value, offset),
                                generateConfig<NtpConfig, String>()
                            )
                                .then {
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

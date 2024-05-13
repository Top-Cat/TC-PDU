package uk.co.thomasc.tcpdu.page.home

import external.Axios
import external.generateConfig
import kotlinx.html.InputType
import kotlinx.html.id
import kotlinx.html.js.onChangeFunction
import kotlinx.html.js.onClickFunction
import kotlinx.serialization.Serializable
import org.w3c.dom.HTMLInputElement
import org.w3c.dom.HTMLSelectElement
import react.Props
import react.dom.a
import react.dom.button
import react.dom.defaultValue
import react.dom.div
import react.dom.hr
import react.dom.i
import react.dom.input
import react.dom.label
import react.dom.option
import react.dom.select
import react.fc
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.page.BootState
import uk.co.thomasc.tcpdu.page.PDUDeviceState
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.util.fixed

external interface OutputProps : Props {
    var idx: Int
    var device: PDUDeviceState
    var callback: (DeviceStateUpdate) -> Unit
}

@Serializable
data class DeviceStateUpdate(
    val idx: Int,
    val state: Boolean? = null,
    val name: String? = null,
    val address: UByte? = null,
    val priority: UByte? = null,
    val bootDelay: UByte? = null,
    val bootState: BootState? = null,
    val maxPower: UShort? = null
)

val output = fc<OutputProps> { props ->
    val dev = props.device
    val history = useNavigate()
    val (loading, setLoading) = useState(false)
    val (editing, setEditing) = useState(false)
    val (bootState, setBootState) = useState(dev.bootState)

    val nameRef = useRef<HTMLInputElement>()
    val priorityRef = useRef<HTMLInputElement>()
    val addressRef = useRef<HTMLInputElement>()
    val bootDelayRef = useRef<HTMLInputElement>()
    val maxPowerRef = useRef<HTMLInputElement>()

    div("card border-primary") {
        div("card-header") {
            if (editing) {
                input(InputType.text, classes = "form-control") {
                    attrs.placeholder = "Output ${props.idx}"
                    attrs.defaultValue = dev.name
                    attrs.id = "output-${props.idx}-name"
                    attrs.disabled = loading
                    attrs.maxLength = "64"
                    ref = nameRef
                }
            } else {
                +dev.name
                div("float-end") {
                    a("#") {
                        attrs.onClickFunction = { ev ->
                            ev.preventDefault()
                            setEditing(true)
                        }
                        i("fas fa-pen") {}
                    }
                }
            }
        }
        div("card-body") {
            if (editing) {
                div("output-grid") {
                    div("form-group w-75") {
                        label("form-label") {
                            attrs.htmlFor = "output-${props.idx}-addr"
                            +"Address"
                        }
                        input(InputType.number, classes = "form-control") {
                            attrs.placeholder = "12"
                            attrs.defaultValue = dev.address.toString()
                            attrs.id = "output-${props.idx}-addr"
                            attrs.disabled = loading
                            ref = addressRef
                        }
                    }
                    div("form-group w-75") {
                        label("form-label") {
                            attrs.htmlFor = "output-${props.idx}-prio"
                            +"Priority"
                        }
                        input(InputType.number, classes = "form-control") {
                            attrs.placeholder = "50"
                            attrs.defaultValue = dev.priority.toString()
                            attrs.id = "output-${props.idx}-prio"
                            attrs.disabled = loading
                            ref = priorityRef
                        }
                    }
                    div("form-group w-75") {
                        label("form-label") {
                            attrs.htmlFor = "output-${props.idx}-delay"
                            +"Boot delay (s)"
                        }
                        input(InputType.number, classes = "form-control") {
                            attrs.placeholder = "5"
                            attrs.defaultValue = dev.bootDelay.toString()
                            attrs.id = "output-${props.idx}-delay"
                            attrs.disabled = loading
                            ref = bootDelayRef
                        }
                    }
                    div("form-group w-75") {
                        label("form-label") {
                            attrs.htmlFor = "output-${props.idx}-maxPower"
                            +"Max Power"
                        }
                        input(InputType.number, classes = "form-control") {
                            attrs.placeholder = "5"
                            attrs.defaultValue = dev.maxPower.toString()
                            attrs.id = "output-${props.idx}-maxPower"
                            attrs.disabled = loading
                            ref = maxPowerRef
                        }
                    }
                }
                div("form-group") {
                    label("form-label") {
                        attrs.htmlFor = "output-${props.idx}-bootState"
                        +"Boot state"
                    }
                    select("form-control") {
                        attrs.id = "output-${props.idx}-bootState"
                        attrs.value = bootState.enc.toString()
                        attrs.onChangeFunction = { ev ->
                            setBootState(BootState.of((ev.target as HTMLSelectElement).value.toLongOrNull()) ?: BootState.LAST)
                        }
                        BootState.entries.forEach {
                            option {
                                attrs.value = it.enc.toString()
                                +it.name
                            }
                        }
                    }
                }
            } else {
                div("stat") {
                    i("fas fa-bolt") {}
                    +"${dev.voltage.fixed(2)}V"
                }
                div("stat") {
                    i("fas fa-copyright") {}
                    +"${dev.current.fixed(2)}A"
                }
                div("stat") {
                    i("fas fa-power-off") {}
                    +"${dev.power.fixed(2)}W"
                }
                div("stat") {
                    +"λ ${(dev.power / dev.va).fixed(2)}"
                }
            }

            hr {}

            if (editing) {
                button(classes = "btn btn-primary") {
                    attrs.onClickFunction = { ev ->
                        ev.preventDefault()
                        setLoading(true)

                        val update = DeviceStateUpdate(
                            props.idx,
                            name = nameRef.current?.value,
                            priority = priorityRef.current?.value?.toUByteOrNull(),
                            address = addressRef.current?.value?.toUByteOrNull(),
                            bootDelay = bootDelayRef.current?.value?.toUByteOrNull(),
                            maxPower = maxPowerRef.current?.value?.toUShortOrNull(),
                            bootState = bootState
                        )
                        Axios.post<String>("$apiRoot/state", update, generateConfig<DeviceStateUpdate, String>()).then {
                            props.callback(update)
                            setEditing(false)
                            setLoading(false)
                        }.handleForbidden(history).catch {
                            setLoading(false)
                        }
                    }

                    +"Save"
                }
                button(classes = "btn btn-danger ms-1") {
                    attrs.onClickFunction = { ev ->
                        ev.preventDefault()
                        setEditing(false)
                    }

                    +"Cancel"
                }
            } else {
                button(classes = "btn btn-${if (dev.state) "danger" else "primary"}") {
                    attrs.onClickFunction = { ev ->
                        ev.preventDefault()
                        setLoading(true)

                        val update = DeviceStateUpdate(props.idx, !dev.state)
                        Axios.post<String>("$apiRoot/state", update, generateConfig<DeviceStateUpdate, String>()).then {
                            props.callback(update)
                            setLoading(false)
                        }.handleForbidden(history).catch {
                            setLoading(false)
                        }
                    }

                    val txt = if (dev.state) "OFF" else "ON"
                    +txt
                }
            }
        }
    }
}

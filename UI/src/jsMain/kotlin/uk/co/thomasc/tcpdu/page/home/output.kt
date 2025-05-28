package uk.co.thomasc.tcpdu.page.home

import external.Axios
import external.generateConfig
import kotlinx.serialization.Serializable
import react.Props
import react.dom.html.ReactHTML.a
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.hr
import react.dom.html.ReactHTML.i
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.dom.html.ReactHTML.option
import react.dom.html.ReactHTML.p
import react.dom.html.ReactHTML.select
import react.router.dom.Link
import react.router.useNavigate
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.BootState
import uk.co.thomasc.tcpdu.page.OutputState
import uk.co.thomasc.tcpdu.page.PDUDeviceState
import uk.co.thomasc.tcpdu.page.handleForbidden
import uk.co.thomasc.tcpdu.util.fixed
import web.cssom.ClassName
import web.html.HTMLInputElement
import web.html.InputType

external interface OutputProps : Props {
    var idx: Int
    var device: PDUDeviceState
    var i2c: List<UByte>?
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
    val maxPower: UShort? = null,
    val minAlarm: UShort? = null,
    val maxAlarm: UShort? = null
)

val output = fcmemo<OutputProps>("Output") { props ->
    val dev = props.device
    val history = useNavigate()
    val (loading, setLoading) = useState(false)
    val (editing, setEditing) = useState(false)
    val (bootState, setBootState) = useState(dev.bootState)
    val (address, setAddress) = useState(dev.address)

    val nameRef = useRef<HTMLInputElement>()
    val priorityRef = useRef<HTMLInputElement>()
    val addressRef = useRef<HTMLInputElement>()
    val bootDelayRef = useRef<HTMLInputElement>()
    val maxPowerRef = useRef<HTMLInputElement>()
    val minAlarmRef = useRef<HTMLInputElement>()
    val maxAlarmRef = useRef<HTMLInputElement>()

    div {
        className = ClassName("card border-primary")
        div {
            className = ClassName("card-header ${if (dev.outputState != OutputState.NORMAL) "bg-warning" else ""}")
            if (editing) {
                input {
                    type = InputType.text
                    className = ClassName("form-control")
                    placeholder = "Output ${props.idx}"
                    defaultValue = dev.name
                    id = "output-${props.idx}-name"
                    disabled = loading
                    maxLength = 64
                    ref = nameRef
                }
            } else {
                +dev.name
                div {
                    className = ClassName("float-end")
                    Link {
                        to = "/calibrate/${props.idx}"
                        i { className = ClassName("fas fa-compass") }
                    }
                    a {
                        href = "#"
                        onClick = { ev ->
                            ev.preventDefault()
                            setEditing(true)
                        }
                        i { className = ClassName("fas fa-pen") }
                    }
                }
            }
        }
        div {
            className = ClassName("card-body")
            if (editing) {
                div {
                    className = ClassName("output-grid")
                    div {
                        className = ClassName("w-75")
                        label {
                            className = ClassName("form-label")
                            htmlFor = "output-${props.idx}-addr"
                            +"Address"
                        }
                        props.i2c?.also { i2c ->
                            select {
                                className = ClassName("form-control")
                                id = "output-${props.idx}-addr"
                                value = address.toString()
                                onChange = { ev ->
                                    setAddress(ev.target.value.toUByteOrNull() ?: 255u)
                                }
                                option {
                                    value = "255"
                                    +"Unassigned"
                                }
                                i2c.forEach {
                                    option {
                                        +it.toString()
                                    }
                                }
                            }
                        } ?: run {
                            input {
                                type = InputType.number
                                className = ClassName("form-control")
                                placeholder = "12"
                                defaultValue = dev.address.toString()
                                id = "output-${props.idx}-addr"
                                disabled = loading
                                ref = addressRef
                            }
                        }
                    }
                    div {
                        className = ClassName("w-75")
                        label {
                            className = ClassName("form-label")
                            htmlFor = "output-${props.idx}-prio"
                            +"Priority"
                        }
                        input {
                            type = InputType.number
                            className = ClassName("form-control")
                            placeholder = "50"
                            defaultValue = dev.priority.toString()
                            id = "output-${props.idx}-prio"
                            disabled = loading
                            ref = priorityRef
                        }
                    }
                    div {
                        className = ClassName("w-75")
                        label {
                            className = ClassName("form-label")
                            htmlFor = "output-${props.idx}-delay"
                            +"Boot delay (s)"
                        }
                        input {
                            type = InputType.number
                            className = ClassName("form-control")
                            placeholder = "5"
                            defaultValue = dev.bootDelay.toString()
                            id = "output-${props.idx}-delay"
                            disabled = loading
                            ref = bootDelayRef
                        }
                    }
                    div {
                        className = ClassName("w-75")
                        label {
                            className = ClassName("form-label")
                            htmlFor = "output-${props.idx}-maxPower"
                            +"Max Power"
                        }
                        input {
                            type = InputType.number
                            className = ClassName("form-control")
                            placeholder = "5"
                            defaultValue = dev.maxPower.toString()
                            id = "output-${props.idx}-maxPower"
                            disabled = loading
                            ref = maxPowerRef
                        }
                    }
                    div {
                        className = ClassName("w-75")
                        label {
                            className = ClassName("form-label")
                            htmlFor = "output-${props.idx}-minAlarm"
                            +"Min Alarm"
                        }
                        input {
                            type = InputType.number
                            className = ClassName("form-control")
                            placeholder = "5"
                            defaultValue = dev.minAlarm.toString()
                            id = "output-${props.idx}-minAlarm"
                            disabled = loading
                            ref = minAlarmRef
                        }
                    }
                    div {
                        className = ClassName("w-75")
                        label {
                            className = ClassName("form-label")
                            htmlFor = "output-${props.idx}-maxAlarm"
                            +"Max Alarm"
                        }
                        input {
                            type = InputType.number
                            className = ClassName("form-control")
                            placeholder = "5"
                            defaultValue = dev.maxAlarm.toString()
                            id = "output-${props.idx}-maxAlarm"
                            disabled = loading
                            ref = maxAlarmRef
                        }
                    }
                }
                div {
                    label {
                        className = ClassName("form-label")
                        htmlFor = "output-${props.idx}-bootState"
                        +"Boot state"
                    }
                    select {
                        className = ClassName("form-control")
                        id = "output-${props.idx}-bootState"
                        value = bootState.enc.toString()
                        onChange = { ev ->
                            setBootState(BootState.of(ev.target.value.toLongOrNull()) ?: BootState.LAST)
                        }
                        BootState.entries.forEach {
                            option {
                                value = it.enc.toString()
                                +it.name
                            }
                        }
                    }
                }
            } else {
                div {
                    className = ClassName("stat")
                    i { className = ClassName("fas fa-bolt") }
                    +"${dev.voltage.fixed(2)}V"
                }
                div {
                    className = ClassName("stat")
                    i { className = ClassName("fas fa-copyright") }
                    +"${dev.current.fixed(2)}A"
                }
                div {
                    className = ClassName("stat")
                    i { className = ClassName("fas fa-power-off") }
                    +"${dev.power.fixed(2)}W"
                }
                div {
                    className = ClassName("stat")
                    +"λ ${(dev.power / dev.va).fixed(2)}"
                }
                when (dev.outputState) {
                    OutputState.ALARM -> p {
                        className = ClassName("mt-2")
                        +"ALARMING"
                    }
                    OutputState.TRIP -> p {
                        className = ClassName("mt-2")
                        +"TRIPPED"
                    }
                    else -> {}
                }
            }

            hr {}

            if (editing) {
                button {
                    className = ClassName("btn btn-primary")
                    onClick = { ev ->
                        ev.preventDefault()
                        setLoading(true)

                        val update = DeviceStateUpdate(
                            props.idx,
                            name = nameRef.current?.value,
                            priority = priorityRef.current?.value?.toUByteOrNull(),
                            address = addressRef.current?.value?.toUByteOrNull() ?: address,
                            bootDelay = bootDelayRef.current?.value?.toUByteOrNull(),
                            maxPower = maxPowerRef.current?.value?.toUShortOrNull(),
                            bootState = bootState,
                            minAlarm = minAlarmRef.current?.value?.toUShortOrNull(),
                            maxAlarm = maxAlarmRef.current?.value?.toUShortOrNull()
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
                button {
                    className = ClassName("btn btn-danger ms-1")
                    onClick = { ev ->
                        ev.preventDefault()
                        setEditing(false)
                    }

                    +"Cancel"
                }
            } else {
                button {
                    className = ClassName("btn btn-${if (dev.state) "danger" else "primary"}")
                    onClick = { ev ->
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

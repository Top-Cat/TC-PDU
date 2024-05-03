package uk.co.thomasc.tcpdu.page

import external.Axios
import external.axiosGet
import external.generateConfig
import kotlinx.html.js.onClickFunction
import kotlinx.serialization.Serializable
import react.Props
import react.dom.button
import react.dom.div
import react.dom.hr
import react.dom.i
import react.fc
import react.router.NavigateFunction
import react.router.useNavigate
import react.useEffectOnce
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.util.EnumAsLongSerializer
import uk.co.thomasc.tcpdu.util.NewNavOption
import uk.co.thomasc.tcpdu.util.fixed
import kotlin.js.Promise

@Serializable
data class PDUState(val power: Float, val devices: List<PDUDeviceState>)

@Serializable
data class PDUDeviceState(
    val name: String,
    val voltage: Float,
    val current: Float,
    val power: Float,
    val kwh: Float,
    val va: Float,
    val address: Int,
    val state: Boolean,
    val bootState: BootState,
    val bootDelay: Int,
    val priority: Int
) {
    fun apply(update: DeviceStateUpdate) = copy(state = update.state ?: state)
}

@Serializable(with = BootState.BootStateSerializer::class)
enum class BootState(val enc: Long) {
    ON(0), LAST(1), OFF(2);

    class BootStateSerializer : EnumAsLongSerializer<BootState>(
        "BootState",
        { it.enc },
        { v -> BootState.entries.first { it.enc == v } }
    )
}

@Serializable
data class DeviceStateUpdate(
    val idx: Int,
    val state: Boolean? = null
)

val homePage = fc<Props> {
    val history = useNavigate()
    val (pduState, setPduState) = useState<PDUState>()

    useEffectOnce {
        axiosGet<PDUState>("$apiRoot/state").then {
            setPduState(it.data)
        }.handleForbidden(history)
    }

    div("row row-cols-1 row-cols-md-2 g-4") {
        pduState?.devices?.forEachIndexed { idx, dev ->
            div("col") {
                div("card border-primary") {
                    div("card-header") {
                        +dev.name
                    }
                    div("card-body") {
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

                        hr {}

                        button(classes = "btn btn-${if (dev.state) "danger" else "primary"}") {
                            attrs.onClickFunction = { ev ->
                                ev.preventDefault()
                                val update = DeviceStateUpdate(idx, !dev.state)
                                Axios.post<String>("$apiRoot/state", update, generateConfig<DeviceStateUpdate, String>()).then {
                                    setPduState {
                                        it?.copy(devices = it.devices.take(idx) + it.devices[idx].apply(update) + it.devices.drop(idx + 1))
                                    }
                                }.handleForbidden(history)
                            }

                            val txt = if (dev.state) "OFF" else "ON"
                            +txt
                        }
                    }
                }
            }
        }
    }
}

fun <T> Promise<T>.handleForbidden(history: NavigateFunction): Promise<Unit> = this.catch {
    // TODO: Check response code
    console.log("handleForbidden", it)
    history.invoke("/login", NewNavOption)
}

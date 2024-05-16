package uk.co.thomasc.tcpdu.page

import external.Axios
import external.AxiosError
import external.axiosGet
import kotlinx.serialization.Serializable
import react.Props
import react.dom.div
import react.fc
import react.router.NavigateFunction
import react.router.useNavigate
import react.useEffectOnce
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.page.home.DeviceStateUpdate
import uk.co.thomasc.tcpdu.page.home.output
import uk.co.thomasc.tcpdu.util.EnumAsLongSerializer
import uk.co.thomasc.tcpdu.util.NewNavOption
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
    val address: UByte,
    val state: Boolean,
    val bootState: BootState,
    val bootDelay: UByte,
    val priority: UByte,
    val maxPower: UShort,
    val minAlarm: UShort,
    val maxAlarm: UShort,
    val outputState: OutputState
) {
    fun apply(update: DeviceStateUpdate) = copy(
        state = update.state ?: state,
        name = update.name ?: name,
        priority = update.priority ?: priority,
        address = update.address ?: address,
        bootState = update.bootState ?: bootState,
        bootDelay = update.bootDelay ?: bootDelay,
        maxPower = update.maxPower ?: maxPower,
        minAlarm = update.minAlarm ?: minAlarm,
        maxAlarm = update.maxAlarm ?: maxAlarm
    )
}

@Serializable(with = BootState.BootStateSerializer::class)
enum class BootState(val enc: Long) {
    ON(0), LAST(1), OFF(2);

    companion object {
        fun of(v: Long?) = BootState.entries.firstOrNull { it.enc == v }
    }

    class BootStateSerializer : EnumAsLongSerializer<BootState>(
        "BootState",
        { it.enc },
        { v -> BootState.entries.first { it.enc == v } }
    )
}

@Serializable(with = OutputState.OutputStateSerializer::class)
enum class OutputState(val enc: Long) {
    NORMAL(0), ALARM(1), TRIP(2);

    companion object {
        fun of(v: Long?) = OutputState.entries.firstOrNull { it.enc == v }
    }

    class OutputStateSerializer : EnumAsLongSerializer<OutputState>(
        "OutputState",
        { it.enc },
        { v -> OutputState.entries.first { it.enc == v } }
    )
}

val homePage = fc<Props> {
    val history = useNavigate()
    val (pduState, setPduState) = useState<PDUState>()

    useEffectOnce {
        axiosGet<PDUState>("$apiRoot/state").then {
            setPduState(it)
        }.handleForbidden(history)
    }

    div("row row-cols-1 row-cols-md-2 g-4") {
        pduState?.devices?.forEachIndexed { idx, dev ->
            div("col") {
                output {
                    attrs.idx = idx
                    attrs.device = dev
                    attrs.callback = { update ->
                        setPduState {
                            it?.copy(devices = it.devices.take(idx) + it.devices[idx].apply(update) + it.devices.drop(idx + 1))
                        }
                    }
                }
            }
        }
    }
}

fun <T> Promise<T>.handleForbidden(history: NavigateFunction): Promise<Unit> = this.catch {
    if (Axios.isAxiosError(it)) {
        val error = it.unsafeCast<AxiosError>()
        console.log("AXIOS ERROR", error.code, error.response?.status)

        if (error.response?.status == 401) {
            history.invoke("/login", NewNavOption)
            return@catch
        }
    }

    throw it
}

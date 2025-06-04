package uk.co.thomasc.tcpdu.page

import external.Axios
import external.AxiosError
import external.axiosGet
import kotlinx.browser.window
import kotlinx.serialization.Serializable
import react.Props
import react.dom.html.ReactHTML.div
import react.router.NavigateFunction
import react.router.useNavigate
import react.useEffectOnceWithCleanup
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.page.home.DeviceStateUpdate
import uk.co.thomasc.tcpdu.page.home.output
import uk.co.thomasc.tcpdu.util.EnumAsLongSerializer
import uk.co.thomasc.tcpdu.util.NewNavOption
import web.cssom.ClassName
import kotlin.js.Promise

@Serializable
data class PDUState(val power: Float, val frequency: Float? = null, val devices: List<PDUDeviceState>)

@Serializable
data class I2cInfo(val count: Int, val devices: List<UByte>)

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

val homePage = fcmemo<Props>("Home") {
    val history = useNavigate()
    val callback = useRef<() -> Unit>()
    val handle = useRef<Int>()
    val (pduState, setPduState) = useState<PDUState>()
    val (i2c, setI2c) = useState<List<UByte>>()

    callback.current = {
        axiosGet<PDUState>("$apiRoot/state").then {
            setPduState(it)
        }.handleForbidden(history)

        handle.current = window.setTimeout({
            callback.current?.invoke()
        }, 10_000)
    }

    useEffectOnceWithCleanup {
        callback.current?.invoke()

        onCleanup {
            handle.current?.let {
                window.clearInterval(it)
            }
        }

        axiosGet<I2cInfo>("$apiRoot/i2c").then {
            setI2c(it.devices)
        }
    }

    div {
        className = ClassName("row row-cols-1 row-cols-md-2 g-4")
        pduState?.devices?.forEachIndexed { idx, dev ->
            div {
                className = ClassName("col")
                output {
                    this.idx = idx
                    device = dev
                    this.i2c = i2c
                    this.callback = { update ->
                        setPduState {
                            it?.copy(devices = it.devices.take(idx) + it.devices[idx].apply(update) + it.devices.drop(idx + 1))
                        }
                    }
                    this.frequency = pduState.frequency
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

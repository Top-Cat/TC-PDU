package uk.co.thomasc.tcpdu.page

import ChartDataset
import ChartJs
import ChartJsBase
import external.axiosGet
import js.objects.jso
import kotlinx.browser.window
import kotlinx.datetime.Clock
import kotlinx.datetime.Instant
import kotlinx.html.id
import kotlinx.html.js.onChangeFunction
import org.w3c.dom.HTMLSelectElement
import react.Props
import react.dom.option
import react.dom.select
import react.fc
import react.useEffectOnce
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot

data class DataPoint(val voltage: Float, val current: Float, val power: Float, val va: Float)
data class TimePoint(val time: Instant, val data: Map<Int, DataPoint>)

enum class GraphType(val min: Int?, val max: Int?, val block: (DataPoint) -> Float) {
    Voltage(null, null, { it.voltage }),
    Current(0, null, { it.current }),
    Power(0, null, { it.power }),
    VA(0, null, { it.va }),
    PF(0, 1, { it.power / it.va })
}

val graphPage = fc<Props> {
    val callback = useRef<() -> Unit>()
    val (data, setData) = useState(listOf<TimePoint>())
    val (names, setNames) = useState(listOf<String>())
    val (graphType, setGraphType) = useState(GraphType.Voltage)

    callback.current = {
        axiosGet<PDUState>("$apiRoot/state").then {
            setData(
                data.plus(
                    TimePoint(
                        Clock.System.now(),
                        it.devices.mapIndexed { idx, state ->
                            idx to DataPoint(state.voltage, state.current, state.power, state.va)
                        }.toMap()
                    )
                ).take(360)
            )
            setNames(it.devices.map { dev -> dev.name })
        }.catch {
            console.log("ERROR", it)
        }
    }

    useEffectOnce {
        ChartJsBase.Chart.register(
            ChartJsBase.CategoryScale,
            ChartJsBase.LinearScale,
            ChartJsBase.PointElement,
            ChartJsBase.LineElement,
            ChartJsBase.Tooltip,
            ChartJsBase.Legend,
            ChartJsBase.Colors
        )

        val handle = window.setInterval({
            callback.current?.invoke()
        }, 5000)

        cleanup {
            window.clearInterval(handle)
        }
    }

    select("form-control") {
        attrs.id = "graph-type"
        attrs.value = graphType.name
        attrs.onChangeFunction = { ev ->
            setGraphType(GraphType.valueOf((ev.target as HTMLSelectElement).value))
        }
        GraphType.entries.forEach {
            option {
                +it.name
            }
        }
    }

    if (data.isNotEmpty()) {
        ChartJs.Line {
            attrs.type = "line"
            attrs.data = jso {
                labels = data.map { it.time.toString() }.toTypedArray()
                datasets = names.mapIndexed { idx, name ->
                    jso<ChartDataset> {
                        label = "$name ${graphType.name}"
                        tension = 0.1f
                        this.data = data.map { it.data[idx]?.let { dp -> graphType.block.invoke(dp) } ?: 0 }.toTypedArray()
                    }
                }.toTypedArray()
            }
            attrs.options = jso {
                scales = jso {
                    y = jso {
                        min = graphType.min
                        max = graphType.max
                    }
                }
            }
        }
    }
}

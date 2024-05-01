import react.ComponentClass
import react.Props

@JsModule("react-chartjs-2")
@JsNonModule
external object ChartJs {
    val Line: ComponentClass<ChartProps>
}

@JsModule("chart.js")
@JsNonModule
external object ChartJsBase {
    val Chart: Chart
    val CategoryScale: dynamic
    val LinearScale: dynamic
    val PointElement: dynamic
    val LineElement: dynamic
    val Tooltip: dynamic
    val Legend: dynamic
    val Colors: dynamic
}

external class Chart {
    fun register(scale: dynamic)
}

external interface ChartProps : Props {
    var type: String
    var data: ChartData
    var options: ChartOptions
}

external interface ChartOptions {
    var scales: ScaleOptions
}

external interface ScaleOptions {
    var x: ChartScaleOptions
    var y: ChartScaleOptions
}

external interface ChartScaleOptions {
    var min: Number?
    var max: Number?
    var reverse: Boolean
    var suggestedMin: Number
    var suggestedMax: Number
    var weight: Number
    var clip: Boolean
    var backgroundColor: String
    var alignToPixels: Boolean
    var display: Boolean
}

external interface ChartData {
    var labels: Array<String>
    var datasets: Array<ChartDataset>
}

external interface ChartDataset {
    var label: String
    var data: Array<Number>
    var fill: Boolean
    var borderColor: String
    var tension: Float
}

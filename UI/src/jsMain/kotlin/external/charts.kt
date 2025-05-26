import org.w3c.dom.HTMLCanvasElement
import react.ComponentType
import react.Props

@JsModule("react-chartjs-2")
@JsNonModule
external object ChartJs {
    val Line: ComponentType<ChartProps>
}

@JsModule("chart.js")
@JsNonModule
external object ChartJsBase {
    val Chart: Chart
    val CategoryScale: ChartComponent
    val LinearScale: ChartComponent
    val PointElement: ChartComponent
    val LineElement: ChartComponent
    val Tooltip: ChartComponent
    val Legend: ChartComponent
    val Colors: ChartComponent
}

external interface ChartComponent {
    var id: String
    var defaults: Any
}

external class Chart {
    val id: String
    val canvas: HTMLCanvasElement
    val width: Number
    val height: Number
    val aspectRatio: Number
    val version: String
    fun register(vararg component: ChartComponent)
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

package external

import react.ComponentType
import react.Props

external interface TimeAgoProps : Props {
    var date: Any
    var minPeriod: Int
    var formatter: (Number, String, String) -> String
}

@JsModule("react-timeago")
@JsNonModule
external object TimeAgo {
    val default: ComponentType<TimeAgoProps>
}

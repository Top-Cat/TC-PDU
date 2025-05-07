package uk.co.thomasc.tcpdu.page.system

import external.Axios
import external.TimeAgo
import external.generateConfig
import js.date.Date
import kotlinx.html.ButtonType
import kotlinx.html.js.onClickFunction
import react.dom.button
import react.dom.div
import react.dom.hr
import react.dom.html.ReactHTML.br
import react.dom.html.ReactHTML.p
import react.dom.i
import react.fc
import uk.co.thomasc.tcpdu.apiRoot

fun bytesToText(b: Long) = (b / 1024.0).asDynamic().toFixed(2)

val sysStatus = fc<SystemProps> { props ->
    props.system?.let { system ->
        div("card border-primary") {
            div("card-header") {
                +"System"
            }
            div("card-body") {
                p {
                    +"Free heap: ${bytesToText(system.mem)} kB"
                    br {}
                    +"FS: ${bytesToText(system.fs.used)} / ${bytesToText(system.fs.total)} kB"
                    br {}
                    +"Uptime: "
                    TimeAgo.default {
                        attrs.date = Date.now() - (system.uptime * 1000)
                        attrs.formatter = { value, u, _ ->
                            val unit = if (value != 1) "${u}s" else u
                            "$value $unit"
                        }
                    }
                    br {}
                    +"Current time: ${system.time}"
                }

                hr {}

                system.temps.forEachIndexed { idx, temp ->
                    if (idx > 0) br {}

                    val color = when {
                        temp < 30 -> "info"
                        temp < 40 -> "success"
                        temp < 60 -> "warning"
                        else -> "danger"
                    }
                    i("fas fa-circle text-$color") {}

                    +" Temp $idx: $temp°C"
                }UI/src/jsMain/kotlin/uk/co/thomasc/tcpdu/page/system/sys.kt

                hr {}

                button(type = ButtonType.submit, classes = "btn btn-primary") {
                    attrs.onClickFunction = { ev ->
                        ev.preventDefault()

                        Axios.post<String>("$apiRoot/reboot", "", generateConfig<String, String>())
                    }
                    +"Reboot"
                }

                button(type = ButtonType.submit, classes = "btn btn-primary ms-2") {
                    attrs.onClickFunction = { ev ->
                        ev.preventDefault()

                        Axios.post<String>("$apiRoot/format", "", generateConfig<String, String>())
                    }
                    +"Clear Logs"
                }
            }
        }
    }
}

package uk.co.thomasc.tcpdu.page.system

import external.Axios
import external.TimeAgo
import external.generateConfig
import js.date.Date
import react.dom.html.ReactHTML
import react.dom.html.ReactHTML.br
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.hr
import react.dom.html.ReactHTML.i
import react.dom.html.ReactHTML.p
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.fcmemo
import web.cssom.ClassName
import web.html.ButtonType

fun bytesToText(b: Long) = (b / 1024.0).asDynamic().toFixed(2)

val sysStatus = fcmemo<SystemProps>("System Status") { props ->
    props.system?.let { system ->
        div {
            className = ClassName("card border-primary")
            div {
                className = ClassName("card-header")
                +"System"
            }
            div {
                className = ClassName("card-body")
                p {
                    +"Free heap: ${bytesToText(system.mem)} kB"
                    br {}
                    +"FS: ${bytesToText(system.fs.used)} / ${bytesToText(system.fs.total)} kB"
                    br {}
                    +"Uptime: "
                    TimeAgo.default {
                        date = Date.now() - (system.uptime * 1000)
                        formatter = { value, u, _ ->
                            val unit = if (value != 1) "${u}s" else u
                            "$value $unit"
                        }
                    }
                    br {}
                    +"Current time: ${system.time}"
                }

                hr {}

                system.temps?.forEachIndexed { idx, temp ->
                    if (idx > 0) br {}

                    val color = when {
                        temp < 30 -> "info"
                        temp < 40 -> "success"
                        temp < 60 -> "warning"
                        else -> "danger"
                    }
                    i { className = ClassName("fas fa-circle text-$color") }

                    +" Temp $idx: $temp°C"
                }

                hr {}

                ReactHTML.button {
                    type = ButtonType.submit
                    className = ClassName("btn btn-primary")
                    onClick = { ev ->
                        ev.preventDefault()

                        Axios.post<String>("$apiRoot/reboot", "", generateConfig<String, String>())
                    }
                    +"Reboot"
                }

                ReactHTML.button {
                    type = ButtonType.submit
                    className = ClassName("btn btn-primary ms-2")
                    onClick = { ev ->
                        ev.preventDefault()

                        Axios.post<String>("$apiRoot/format", "", generateConfig<String, String>())
                    }
                    +"Clear Logs"
                }
            }
        }
    }
}

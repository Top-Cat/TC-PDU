package uk.co.thomasc.tcpdu.page.system

import external.TimeAgo
import js.date.Date
import react.dom.div
import react.dom.html.ReactHTML.br
import react.dom.html.ReactHTML.p
import react.fc

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
            }
        }
    }
}

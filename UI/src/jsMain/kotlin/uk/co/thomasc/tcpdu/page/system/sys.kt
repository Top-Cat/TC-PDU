package uk.co.thomasc.tcpdu.page.system

import external.TimeAgo
import js.date.Date
import react.dom.div
import react.dom.html.ReactHTML.br
import react.dom.html.ReactHTML.p
import react.fc

val sysStatus = fc<SystemProps> { props ->
    props.system?.let { system ->
        div("card border-primary") {
            div("card-header") {
                +"System"
            }
            div("card-body") {
                p {
                    +"Free heap: ${(system.mem / 1024.0).asDynamic().toFixed(2)} kB"
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

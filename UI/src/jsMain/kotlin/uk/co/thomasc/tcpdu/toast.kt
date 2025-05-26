package uk.co.thomasc.tcpdu

import js.objects.jso
import react.PropsWithChildren
import react.dom.aria.AriaRole
import react.dom.html.ReactHTML.div
import react.dom.svg.ReactSVG.path
import react.dom.svg.ReactSVG.svg
import web.cssom.ClassName
import web.cssom.px

val success = fcmemo<PropsWithChildren>("Success Toast") {
    div {
        role = AriaRole.alert
        className = ClassName("alert alert-success d-flex align-items-center")
        svg {
            role = AriaRole.img
            className = ClassName("bi flex-shrink-0 me-2")
            viewBox = "0 0 16 16"
            style = jso {
                width = 24.px
                height = 24.px
            }
            path {
                fill = "currentColor"
                d = "M16 8A8 8 0 1 1 0 8a8 8 0 0 1 16 0zm-3.97-3.03a.75.75 0 0 0-1.08.022L7.477 9.417 5.384 " +
                    "7.323a.75.75 0 0 0-1.06 1.06L6.97 11.03a.75.75 0 0 0 1.079-.02l3.992-4.99a.75.75 0 0 0-.01-1.05z"
            }
        }
        div {
            +it.children
        }
    }
}

val errors = fcmemo<PropsWithChildren>("Error Toast") {
    div {
        role = AriaRole.alert
        className = ClassName("alert alert-danger d-flex align-items-center")
        svg {
            role = AriaRole.img
            className = ClassName("bi flex-shrink-0 me-2")
            viewBox = "0 0 16 16"
            style = jso {
                width = 24.px
                height = 24.px
            }
            path {
                fill = "currentColor"
                d = "M8.982 1.566a1.13 1.13 0 0 0-1.96 0L.165 13.233c-.457.778.091 1.767.98 1.767h13.713c.889 0 1.438-.99.98-1.767L8.982 " +
                    "1.566zM8 5c.535 0 .954.462.9.995l-.35 3.507a.552.552 0 0 1-1.1 0L7.1 5.995A.905.905 0 0 1 8 5zm.002 6a1 1 0 1 1 0 2 1 1 0 0 1 0-2z"
            }
        }
        div {
            +it.children
        }
    }
}

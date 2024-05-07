package uk.co.thomasc.tcpdu

import kotlinx.html.role
import react.PropsWithChildren
import react.dom.div
import react.dom.jsStyle
import react.dom.svg
import react.dom.svg.ReactSVG.path
import react.fc

val success = fc<PropsWithChildren> {
    div("alert alert-success d-flex align-items-center") {
        attrs.role = "alert"
        svg("bi flex-shrink-0 me-2") {
            attrs.role = "img"
            attrs.attributes["viewBox"] = "0 0 16 16"
            attrs.jsStyle {
                width = "24px"
                height = "24px"
            }
            path {
                attrs.fill = "currentColor"
                attrs.d = "M16 8A8 8 0 1 1 0 8a8 8 0 0 1 16 0zm-3.97-3.03a.75.75 0 0 0-1.08.022L7.477 9.417 5.384 " +
                    "7.323a.75.75 0 0 0-1.06 1.06L6.97 11.03a.75.75 0 0 0 1.079-.02l3.992-4.99a.75.75 0 0 0-.01-1.05z"
            }
        }
        div {
            it.children()
        }
    }
}

val errors = fc<PropsWithChildren> {
    div("alert alert-danger d-flex align-items-center") {
        attrs.role = "alert"
        svg("bi flex-shrink-0 me-2") {
            attrs.role = "img"
            attrs.attributes["viewBox"] = "0 0 16 16"
            attrs.jsStyle {
                width = "24px"
                height = "24px"
            }
            path {
                attrs.fill = "currentColor"
                attrs.d = "M8.982 1.566a1.13 1.13 0 0 0-1.96 0L.165 13.233c-.457.778.091 1.767.98 1.767h13.713c.889 0 1.438-.99.98-1.767L8.982 " +
                    "1.566zM8 5c.535 0 .954.462.9.995l-.35 3.507a.552.552 0 0 1-1.1 0L7.1 5.995A.905.905 0 0 1 8 5zm.002 6a1 1 0 1 1 0 2 1 1 0 0 1 0-2z"
            }
        }
        div {
            it.children()
        }
    }
}

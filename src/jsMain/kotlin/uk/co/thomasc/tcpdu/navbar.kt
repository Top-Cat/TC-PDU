package uk.co.thomasc.tcpdu

import kotlinx.browser.window
import kotlinx.html.js.onClickFunction
import org.w3c.dom.HTMLElement
import react.Props
import react.dom.button
import react.dom.div
import react.dom.jsStyle
import react.dom.li
import react.dom.nav
import react.dom.span
import react.dom.ul
import react.fc
import react.router.dom.Link
import react.router.useLocation
import react.useEffect
import react.useRef
import react.useState
import web.cssom.ClassName

val navbar = fc<Props> {
    val location = useLocation()
    val currentPath = location.pathname
    val (expanded, setExpanded) = useState(false)
    val (expanding, setExpanding) = useState<Boolean?>(null)
    val (height, setHeight) = useState<Int?>(null)
    val navRef = useRef<HTMLElement>()

    useEffect(currentPath) {
        setExpanded(false)
    }

    useEffect(height) {
        if (expanding != null) {
            setHeight(if (expanding == true) navRef.current?.scrollHeight ?: 0 else 0)

            val handle = window.setTimeout({
                setExpanded(!expanded)
                setExpanding(null)
                setHeight(null)
            }, 500)

            cleanup {
                window.clearTimeout(handle)
            }
        }
    }

    nav("navbar navbar-dark navbar-expand-lg fixed-top bg-primary") {
        div("container") {
            Link {
                attrs.to = "/"
                attrs.className = ClassName("navbar-brand")
                +"TC-PDU"
            }
            button(classes = "navbar-toggler") {
                attrs.onClickFunction = { ev ->
                    ev.preventDefault()

                    setExpanding(!expanded)
                    setHeight(if (!expanded) 0 else navRef.current?.scrollHeight ?: 0)
                }
                attrs.attributes["data-toggle"] = "collapse"
                attrs.attributes["data-target"] = "navbar"
                attrs.attributes["aria-controls"] = "navbar"
                attrs.attributes["aria-expanded"] = "false"
                attrs.attributes["aria-label"] = "Toggle navigation"
                span("navbar-toggler-icon") {}
            }

            div("navbar-collapse${if (expanded) " show" else ""} ${if (expanding != null) "collapsing" else "collapse"}") {
                if (height != null) {
                    attrs.jsStyle {
                        this.height = "${height}px"
                    }
                }
                ref = navRef

                ul("navbar-nav me-auto") {
                    li("nav-item") {
                        Link {
                            attrs.to = "/graph"
                            attrs.className = ClassName("nav-link${if (currentPath == "/graph") " active" else ""}")
                            +"Graph"
                        }
                    }
                    li("nav-item") {
                        Link {
                            attrs.to = "/config"
                            attrs.className = ClassName("nav-link${if (currentPath == "/config") " active" else ""}")
                            +"Config"
                        }
                    }
                    li("nav-item") {
                        Link {
                            attrs.to = "/logs"
                            attrs.className = ClassName("nav-link${if (currentPath == "/logs") " active" else ""}")
                            +"Logs"
                        }
                    }
                    li("nav-item") {
                        Link {
                            attrs.to = "/system"
                            attrs.className = ClassName("nav-link${if (currentPath == "/system") " active" else ""}")
                            +"System"
                        }
                    }
                    li("nav-item") {
                        Link {
                            attrs.to = "/fw"
                            attrs.className = ClassName("nav-link${if (currentPath == "/fw") " active" else ""}")
                            +"Firmware"
                        }
                    }
                }
            }
        }
    }
}

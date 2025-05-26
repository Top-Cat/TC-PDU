package uk.co.thomasc.tcpdu

import js.objects.jso
import kotlinx.browser.window
import react.Props
import react.dom.html.HTMLAttributes
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.li
import react.dom.html.ReactHTML.nav
import react.dom.html.ReactHTML.span
import react.dom.html.ReactHTML.ul
import react.router.dom.Link
import react.router.useLocation
import react.useEffect
import react.useEffectWithCleanup
import react.useRef
import react.useState
import web.cssom.ClassName
import web.cssom.px
import web.html.HTMLElement

var HTMLAttributes<*>.dataToggle: String
    get() = asDynamic()["data-toggle"]
    set(value) {
        asDynamic()["data-toggle"] = value
    }

var HTMLAttributes<*>.dataTarget: String
    get() = asDynamic()["data-target"]
    set(value) {
        asDynamic()["data-target"] = value
    }

val navbar = fcmemo<Props>("Navbar") {
    val location = useLocation()
    val currentPath = location.pathname
    val (expanded, setExpanded) = useState(false)
    val (expanding, setExpanding) = useState<Boolean?>(null)
    val (height, setHeight) = useState<Int?>(null)
    val navRef = useRef<HTMLElement>()

    useEffect(currentPath) {
        setExpanded(false)
    }

    useEffectWithCleanup(height) {
        if (expanding != null) {
            setHeight(if (expanding == true) navRef.current?.scrollHeight ?: 0 else 0)

            val handle = window.setTimeout({
                setExpanded(!expanded)
                setExpanding(null)
                setHeight(null)
            }, 500)

            onCleanup {
                window.clearTimeout(handle)
            }
        }
    }

    nav {
        className = ClassName("navbar navbar-dark navbar-expand-lg fixed-top bg-primary")
        div {
            className = ClassName("container")
            Link {
                to = "/"
                className = ClassName("navbar-brand")
                +"TC-PDU"
            }
            button {
                className = ClassName("navbar-toggler")
                onClick = { ev ->
                    ev.preventDefault()

                    setExpanding(!expanded)
                    setHeight(if (!expanded) 0 else navRef.current?.scrollHeight ?: 0)
                }
                ariaControls = "navbar"
                ariaExpanded = false
                ariaLabel = "Toggle navigation"
                dataToggle = "collapse"
                dataTarget = "navbar"
                span { className = ClassName("navbar-toggler-icon") }
            }

            div {
                className = ClassName("navbar-collapse${if (expanded) " show" else ""} ${if (expanding != null) "collapsing" else "collapse"}")
                if (height != null) {
                    style = jso {
                        this.height = height.px
                    }
                }
                ref = navRef

                ul {
                    className = ClassName("navbar-nav me-auto")
                    li {
                        className = ClassName("nav-item")
                        Link {
                            to = "/graph"
                            className = ClassName("nav-link${if (currentPath == "/graph") " active" else ""}")
                            +"Graph"
                        }
                    }
                    li {
                        className = ClassName("nav-item")
                        Link {
                            to = "/config"
                            className = ClassName("nav-link${if (currentPath == "/config") " active" else ""}")
                            +"Config"
                        }
                    }
                    li {
                        className = ClassName("nav-item")
                        Link {
                            to = "/logs"
                            className = ClassName("nav-link${if (currentPath == "/logs") " active" else ""}")
                            +"Logs"
                        }
                    }
                    li {
                        className = ClassName("nav-item")
                        Link {
                            to = "/system"
                            className = ClassName("nav-link${if (currentPath == "/system") " active" else ""}")
                            +"System"
                        }
                    }
                    li {
                        className = ClassName("nav-item")
                        Link {
                            to = "/fw"
                            className = ClassName("nav-link${if (currentPath == "/fw") " active" else ""}")
                            +"Firmware"
                        }
                    }
                }
            }
        }
    }
}

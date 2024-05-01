package uk.co.thomasc.tcpdu

import external.jsJodaTz
import js.objects.jso
import kotlinx.browser.window
import kotlinx.html.id
import react.Props
import react.createElement
import react.dom.client.createRoot
import react.dom.div
import react.fc
import react.router.Outlet
import react.router.RouterProvider
import react.router.dom.createBrowserRouter
import uk.co.thomasc.tcpdu.page.configPage
import uk.co.thomasc.tcpdu.page.firmwarePage
import uk.co.thomasc.tcpdu.page.graphPage
import uk.co.thomasc.tcpdu.page.homePage
import uk.co.thomasc.tcpdu.page.loginPage
import uk.co.thomasc.tcpdu.page.logsPage
import uk.co.thomasc.tcpdu.page.systemPage
import web.dom.document

const val apiRoot = "/proxy"

fun main() {
    jsJodaTz
    window.onload = {
        document.getElementById("root")?.let { root ->
            createRoot(root).render(createElement(app))
        }
    }
}

val root = fc<Props> {
    navbar {}
    Outlet()
}

val app = fc<Props> {
    val appRouter = createBrowserRouter(
        arrayOf(
            jso {
                path = "/"
                element = createElement(root)
                children = arrayOf(
                    jso {
                        index = true
                        element = createElement(homePage)
                    },
                    jso {
                        path = "graph"
                        element = createElement(graphPage)
                    },
                    jso {
                        path = "config"
                        element = createElement(configPage)
                    },
                    jso {
                        path = "logs"
                        element = createElement(logsPage)
                    },
                    jso {
                        path = "system"
                        element = createElement(systemPage)
                    },
                    jso {
                        path = "fw"
                        element = createElement(firmwarePage)
                    },
                    jso {
                        path = "login"
                        element = createElement(loginPage)
                    },
                    jso {
                        path = "*"
                        element = createElement(notFound)
                    }
                )
            }
        )
    )

    RouterProvider {
        attrs.router = appRouter
    }
}

val notFound = fc<Props> {
    div {
        attrs.id = "notfound"
        +"Not found"
    }
}

package uk.co.thomasc.tcpdu

import js.objects.jso
import kotlinx.browser.window
import react.ChildrenBuilder
import react.FC
import react.Props
import react.createElement
import react.dom.client.createRoot
import react.dom.html.ReactHTML.div
import react.memo
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

const val apiRoot = "/api"

fun <T : Props> fcmemo(name: String, block: ChildrenBuilder.(props: T) -> Unit) = memo(FC(name, block))

fun main() {
    window.onload = {
        document.getElementById("root")?.let { root ->
            createRoot(root).render(createElement(app))
        }
    }
}

val root = fcmemo<Props>("PDU Root") {
    navbar {}
    Outlet()
}

val app = fcmemo<Props>("PDU App") {
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
        router = appRouter
    }
}

val notFound = fcmemo<Props>("Not Found") {
    div {
        id = "notfound"
        +"Not found"
    }
}

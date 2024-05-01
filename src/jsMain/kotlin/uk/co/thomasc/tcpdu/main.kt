package uk.co.thomasc.tcpdu

import external.jsJodaTz
import kotlinx.browser.document
import kotlinx.browser.window
import kotlinx.html.id
import react.Props
import react.createElement
import react.dom.div
import react.dom.render
import react.fc
import react.router.Route
import react.router.Routes
import react.router.dom.BrowserRouter
import uk.co.thomasc.tcpdu.page.configPage
import uk.co.thomasc.tcpdu.page.firmwarePage
import uk.co.thomasc.tcpdu.page.graphPage
import uk.co.thomasc.tcpdu.page.homePage
import uk.co.thomasc.tcpdu.page.loginPage
import uk.co.thomasc.tcpdu.page.logsPage
import uk.co.thomasc.tcpdu.page.systemPage

const val apiRoot = "/proxy"

fun main() {
    jsJodaTz
    window.onload = {
        document.getElementById("root")?.let { root ->
            render(root) {
                app { }
            }
        }
    }
}

val app = fc<Props> {
    BrowserRouter {
        navbar {}
        Routes {
            Route {
                attrs.path = "/"
                attrs.element = createElement(homePage)
            }
            Route {
                attrs.path = "/graph"
                attrs.element = createElement(graphPage)
            }
            Route {
                attrs.path = "/config"
                attrs.element = createElement(configPage)
            }
            Route {
                attrs.path = "/logs"
                attrs.element = createElement(logsPage)
            }
            Route {
                attrs.path = "/system"
                attrs.element = createElement(systemPage)
            }
            Route {
                attrs.path = "/fw"
                attrs.element = createElement(firmwarePage)
            }
            Route {
                attrs.path = "/login"
                attrs.element = createElement(loginPage)
            }
            Route {
                attrs.path = "*"
                attrs.element = createElement(notFound)
            }
        }
    }
}

val notFound = fc<Props> {
    div {
        attrs.id = "notfound"
        +"Not found"
    }
}

package uk.co.thomasc.tcpdu.page

import external.Axios
import external.AxiosProgress
import external.AxiosRequestConfig
import external.axiosGet
import kotlinx.html.ButtonType
import kotlinx.html.InputType
import kotlinx.html.hidden
import kotlinx.html.id
import kotlinx.html.js.onClickFunction
import kotlinx.html.role
import org.w3c.dom.HTMLElement
import org.w3c.dom.HTMLInputElement
import org.w3c.files.get
import org.w3c.xhr.FormData
import react.Props
import react.dom.button
import react.dom.div
import react.dom.form
import react.dom.input
import react.dom.label
import react.dom.p
import react.fc
import react.router.useNavigate
import react.useEffectOnce
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot

class UploadRequestConfig(block: (AxiosProgress) -> Unit) : AxiosRequestConfig {
    override var onUploadProgress: ((progressEvent: AxiosProgress) -> Unit)? = block
    override var validateStatus: ((Number) -> Boolean)? = {
        arrayOf(200, 400, 401, 413).contains(it)
    }
}

val firmwarePage = fc<Props> {
    val history = useNavigate()
    val (curFw, setFw) = useState("Unknown")
    val (loading, setLoading) = useState(false)
    val fwRef = useRef<HTMLInputElement>()
    val progressRef = useRef<HTMLElement>()

    useEffectOnce {
        axiosGet<PDUSystem>("$apiRoot/system").then {
            setFw(it.data.fw)
        }.handleForbidden(history)
    }

    form(classes = "w-50 m-auto") {
        p { +"Current firmware: $curFw" }

        div("progress") {
            attrs.hidden = !loading
            div("progress-bar progress-bar-striped progress-bar-animated bg-info") {
                attrs.role = "progressbar"
                ref = progressRef
            }
        }

        div("form-group mt-3") {
            label("form-label") {
                attrs.htmlFor = "fw-file"
                +"New firmware"
            }
            input(InputType.file, classes = "form-control") {
                attrs.placeholder = "firmware.bin"
                attrs.id = "fw-file"
                attrs.disabled = loading
                ref = fwRef
            }
        }

        button(type = ButtonType.submit, classes = "btn btn-primary w-100 mt-3") {
            attrs.disabled = loading
            attrs.onClickFunction = { ev ->
                ev.preventDefault()

                setLoading(true)
                val data = FormData()
                data.asDynamic().append("file", fwRef.current?.files?.get(0))

                Axios.post<String>(
                    "/update",
                    data,
                    UploadRequestConfig { progress ->
                        val v = ((progress.loaded * 100f) / progress.total).toInt()
                        progressRef.current?.style?.width = "$v%"
                    }
                ).then {
                    // TODO: Show toast
                    console.log("Success?")
                    setLoading(false)
                }.handleForbidden(history).catch { setLoading(false) }
            }
            +"Upgrade"
        }
    }
}

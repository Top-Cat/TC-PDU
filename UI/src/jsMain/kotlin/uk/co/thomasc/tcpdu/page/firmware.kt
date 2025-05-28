package uk.co.thomasc.tcpdu.page

import external.Axios
import external.AxiosProgress
import external.AxiosRequestConfig
import external.axiosGet
import react.Props
import react.dom.aria.AriaRole
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.form
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.dom.html.ReactHTML.p
import react.router.useNavigate
import react.useEffectOnce
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.fcmemo
import web.cssom.ClassName
import web.form.FormData
import web.html.ButtonType
import web.html.HTMLElement
import web.html.HTMLInputElement
import web.html.InputType

class UploadRequestConfig(block: (AxiosProgress) -> Unit) : AxiosRequestConfig {
    override var onUploadProgress: ((progressEvent: AxiosProgress) -> Unit)? = block
    override var validateStatus: ((Number) -> Boolean)? = {
        arrayOf(200, 400, 401, 413).contains(it)
    }
}

val firmwarePage = fcmemo<Props>("Firmware") {
    val history = useNavigate()
    val (curFw, setFw) = useState("Unknown")
    val (loading, setLoading) = useState(false)
    val fwRef = useRef<HTMLInputElement>()
    val progressRef = useRef<HTMLElement>()

    useEffectOnce {
        axiosGet<PDUSystem>("$apiRoot/system").then {
            setFw(it.fw)
        }.handleForbidden(history)
    }

    form {
        className = ClassName("w-50 m-auto")
        p { +"Current firmware: $curFw" }

        div {
            className = ClassName("progress")
            hidden = !loading
            div {
                className = ClassName("progress-bar progress-bar-striped progress-bar-animated bg-info")
                role = AriaRole.progressbar
                ref = progressRef
            }
        }

        div {
            className = ClassName("mt-3")
            label {
                className = ClassName("form-label")
                htmlFor = "fw-file"
                +"New firmware"
            }
            input {
                type = InputType.file
                className = ClassName("form-control")
                placeholder = "firmware.bin"
                id = "fw-file"
                disabled = loading
                ref = fwRef
            }
        }

        button {
            type = ButtonType.submit
            className = ClassName("btn btn-primary w-100 mt-3")
            disabled = loading
            onClick = { ev ->
                ev.preventDefault()

                setLoading(true)
                val data = FormData()
                fwRef.current?.files?.get(0)?.let { data.append("file", it) }

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

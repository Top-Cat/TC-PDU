package uk.co.thomasc.tcpdu.page

import external.Axios
import external.axiosGet
import external.generateConfig
import kotlinx.browser.window
import kotlinx.serialization.Serializable
import react.Props
import react.dom.html.ReactHTML.button
import react.dom.html.ReactHTML.div
import react.dom.html.ReactHTML.form
import react.dom.html.ReactHTML.hr
import react.dom.html.ReactHTML.i
import react.dom.html.ReactHTML.input
import react.dom.html.ReactHTML.label
import react.router.dom.Link
import react.router.useParams
import react.useEffect
import react.useEffectOnceWithCleanup
import react.useRef
import react.useState
import uk.co.thomasc.tcpdu.apiRoot
import uk.co.thomasc.tcpdu.fcmemo
import uk.co.thomasc.tcpdu.util.fixedStr
import web.cssom.ClassName
import web.html.ButtonType
import web.html.HTMLInputElement
import web.html.InputType

@Serializable
data class CalibrationData(val idx: Int, val current: Float? = null, val voltage: Float? = null, val currentDirect: Float? = null, val voltageDirect: Float? = null) {
    companion object {
        fun reset(idx: Int) = CalibrationData(idx, currentDirect =  1f, voltageDirect = 1f)
    }
}

val calibratePage = fcmemo<Props>("Calibrate") {
    val callback = useRef<() -> Unit>()
    val (current, setCurrent) = useState(listOf<Float>())
    val (voltage, setVoltage) = useState(listOf<Float>())
    val params = useParams()
    val idx = params["idx"]?.toInt() ?: 0

    val avgVoltage = voltage.average().takeIf { !it.isNaN() } ?: 0.0
    val avgCurrent = current.average().takeIf { !it.isNaN() } ?: 0.0

    val voltageRef = useRef<HTMLInputElement>()
    val voltageMultRef = useRef<HTMLInputElement>()
    val currentRef = useRef<HTMLInputElement>()
    val currentMultRef = useRef<HTMLInputElement>()

    val voltageTypeRef = useRef(false)
    val currentTypeRef = useRef(false)

    fun updateVoltage() {
        if (voltageTypeRef.current == true) {
            voltageMultRef.current?.value = ((voltageRef.current?.value?.toFloatOrNull() ?: 240f) / avgVoltage).fixedStr(5)
        } else {
            voltageRef.current?.value = (avgVoltage * (voltageMultRef.current?.value?.toFloatOrNull() ?: 1f)).fixedStr(5)
        }
    }

    fun updateCurrent() {
        if (currentTypeRef.current == true) {
            currentMultRef.current?.value = ((currentRef.current?.value?.toFloatOrNull() ?: 1f) / avgVoltage).fixedStr(5)
        } else {
            currentRef.current?.value = (avgVoltage * (currentMultRef.current?.value?.toFloatOrNull() ?: 1f)).fixedStr(5)
        }
    }

    useEffect(current, voltage) {
        updateCurrent()
        updateVoltage()
    }

    callback.current = {
        axiosGet<PDUState>("$apiRoot/state").then {
            setCurrent(current.plus(it.devices[idx].current))
            setVoltage(voltage.plus(it.devices[idx].voltage))
        }.catch {
            console.log("ERROR", it)
        }
    }

    useEffectOnceWithCleanup {
        val handle = window.setInterval({
            callback.current?.invoke()
        }, 5000)

        onCleanup {
            window.clearInterval(handle)
        }
    }

    div {
        className = ClassName("card border-dark w-50 m-auto")
        div {
            className = ClassName("card-header")
            +"Calibrating output $idx"

            div {
                className = ClassName("float-end")
                Link {
                    to = "/"
                    i { className = ClassName("fas fa-times") }
                }
            }
        }
        form {
            className = ClassName("card-body")
            div {
                label {
                    className = ClassName("form-label")
                    +"Average voltage"
                }
                div {
                    className = ClassName("mx-2 mb-2")
                    +"$avgVoltage (${voltage.count()})"
                }
            }
            div {
                label {
                    className = ClassName("form-label")
                    htmlFor = "true-voltage"
                    +"True Voltage"
                }
                input {
                    type = InputType.number
                    className = ClassName("form-control")
                    placeholder = "240"
                    id = "true-voltage"
                    ref = voltageRef
                    onChange = {
                        voltageTypeRef.current = true
                        updateVoltage()
                    }
                }
            }
            div {
                label {
                    className = ClassName("form-label")
                    htmlFor = "mult-voltage"
                    +"Multiplier"
                }
                input {
                    type = InputType.number
                    className = ClassName("form-control")
                    placeholder = "1"
                    id = "mult-voltage"
                    ref = voltageMultRef
                    onChange = {
                        voltageTypeRef.current = false
                        updateVoltage()
                    }
                }
            }
            hr {}
            div {
                label {
                    className = ClassName("form-label")
                    +"Average current"
                }
                div {
                    className = ClassName("mx-2 mb-2")
                    +"$avgCurrent (${current.count()})"
                }
            }
            div {
                label {
                    className = ClassName("form-label")
                    htmlFor = "true-current"
                    +"True Current"
                }
                input {
                    type = InputType.number
                    className = ClassName("form-control")
                    placeholder = "1.34"
                    id = "true-current"
                    ref = currentRef
                    onChange = {
                        currentTypeRef.current = true
                        updateCurrent()
                    }
                }
            }
            div {
                label {
                    className = ClassName("form-label")
                    htmlFor = "mult-current"
                    +"Multiplier"
                }
                input {
                    type = InputType.number
                    className = ClassName("form-control")
                    placeholder = "1"
                    id = "mult-current"
                    ref = currentMultRef
                    onChange = {
                        currentTypeRef.current = false
                        updateCurrent()
                    }
                }
            }
            hr {}
            button {
                type = ButtonType.submit
                className = ClassName("btn btn-primary")
                onClick = { ev ->
                    ev.preventDefault()

                    Axios.post<String>("$apiRoot/calibrate", CalibrationData(idx, currentMultRef.current?.value?.toFloatOrNull(), voltageMultRef.current?.value?.toFloatOrNull()), generateConfig<CalibrationData, String>()).then {
                        setCurrent(listOf())
                        setVoltage(listOf())
                        currentMultRef.current?.value = "1"
                        voltageMultRef.current?.value = "1"
                    }
                }
                +"Update Calibration"
            }
            button {
                type = ButtonType.submit
                className = ClassName("btn btn-primary ms-2")
                onClick = { ev ->
                    ev.preventDefault()

                    setCurrent(listOf())
                    setVoltage(listOf())
                }
                +"Reset Stats"
            }
            button {
                type = ButtonType.submit
                className = ClassName("btn btn-primary ms-2")
                onClick = { ev ->
                    ev.preventDefault()

                    Axios.post<String>("$apiRoot/calibrate", CalibrationData.reset(idx), generateConfig<CalibrationData, String>()).then {
                        setCurrent(listOf())
                        setVoltage(listOf())
                        currentMultRef.current?.value = "1"
                        voltageMultRef.current?.value = "1"
                    }
                }
                +"Reset Calibration"
            }
        }
    }
}

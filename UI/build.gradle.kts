import io.miret.etienne.gradle.sass.CompileSass
import org.jetbrains.kotlin.gradle.dsl.JvmTarget
import org.jetbrains.kotlin.gradle.targets.js.webpack.KotlinWebpack
import org.jlleitschuh.gradle.ktlint.reporter.ReporterType

val kotlinVersion: String by project
val ktorVersion: String by project

plugins {
    kotlin("multiplatform") version "2.1.0"
    kotlin("plugin.serialization") version "2.1.0"
    id("io.miret.etienne.sass") version "1.1.2"
    id("org.jlleitschuh.gradle.ktlint") version "11.5.1"
    application
}

group = "uk.co.thomasc"

repositories {
    mavenCentral()
    maven { url = uri("https://artifactory.kirkstall.top-cat.me") }
}

kotlin {
    jvmToolchain {
        languageVersion.set(JavaLanguageVersion.of(21))
    }
    jvm {
        compilerOptions.jvmTarget = JvmTarget.JVM_21
        testRuns["test"].executionTask.configure {
            useJUnitPlatform()
        }
        withJava()
    }
    js(IR) {
        browser {
            binaries.executable()
            webpackTask {
                cssSupport {
                    enabled.set(true)
                }
            }
            runTask {
                cssSupport {
                    enabled.set(true)
                }
            }
            testTask {
                useKarma {
                    useChromeHeadless()
                    webpackConfig.cssSupport {
                        enabled.set(true)
                    }
                }
            }
        }
    }
    sourceSets {
        val jvmMain by getting {
            with(languageSettings) {
                optIn("kotlin.io.path.ExperimentalPathApi")
                optIn("io.ktor.server.locations.KtorExperimentalLocationsAPI")
            }

            dependencies {
                implementation("io.ktor:ktor-client-content-negotiation:$ktorVersion")
                implementation("io.ktor:ktor-server-content-negotiation:$ktorVersion")
                implementation("io.ktor:ktor-serialization-kotlinx-json:$ktorVersion")

                implementation("io.ktor:ktor-server-auth:$ktorVersion")
                implementation("io.ktor:ktor-server-mustache:$ktorVersion")
                implementation("io.ktor:ktor-server-netty:$ktorVersion")
                implementation("io.ktor:ktor-server-html-builder:$ktorVersion")
                implementation("io.ktor:ktor-server-status-pages:$ktorVersion")
                implementation("org.jetbrains.kotlinx:kotlinx-html-jvm:0.8.0")
                implementation("ch.qos.logback:logback-classic:1.4.7")
                implementation("io.ktor:ktor-server-locations:$ktorVersion")

                implementation("io.ktor:ktor-client-apache:$ktorVersion")
            }
        }
        val jsMain by getting {
            with(languageSettings) {
                optIn("kotlin.js.ExperimentalJsExport")
                optIn("kotlin.time.ExperimentalTime")
                optIn("kotlinx.serialization.ExperimentalSerializationApi")
                optIn("kotlin.io.encoding.ExperimentalEncodingApi")
            }
            dependencies {
                implementation("org.jetbrains.kotlinx:kotlinx-datetime:0.6.0-RC.2")
                implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.6.1")

                implementation("org.jetbrains.kotlin-wrappers:kotlin-extensions:1.0.1-pre.736")
                implementation("org.jetbrains.kotlin-wrappers:kotlin-react:19.0.0-pre.860")
                implementation("org.jetbrains.kotlin-wrappers:kotlin-react-dom:19.0.0-pre.860")
                implementation("org.jetbrains.kotlin-wrappers:kotlin-react-router-dom:6.28.0-pre.860")
                implementation(npm("axios", "0.21.1"))
                implementation(npm("bootswatch", "5.3.3"))
                implementation(npm("bootstrap", "5.3.3"))
                implementation(npm("chart.js", "4.4.2"))
                implementation(npm("react-chartjs-2", "5.2.0"))
                implementation(npm("react-timeago", "7.2.0"))
                implementation(devNpm("webpack-bundle-analyzer", "4.6.1"))
            }
        }
        val jsTest by getting {
            dependencies {
                implementation(kotlin("test-common"))
                implementation(kotlin("test-annotations-common"))

                implementation(kotlin("test-js"))
            }
        }
    }
}

application {
    mainClass.set("uk.co.thomasc.tcpdu.MainKt")
}

ktlint {
    version.set("0.50.0")
    reporters {
        reporter(ReporterType.CHECKSTYLE)
    }
}

tasks.getByName<CompileSass>("compileSass") {
    dependsOn(tasks.getByName("kotlinNpmInstall"))

    setSourceDir(file("$projectDir/src/jsMain/sass"))
    loadPath(layout.buildDirectory.file("js/node_modules").get().asFile)
    outputDir = layout.buildDirectory.file("processedResources/jvm/main/static").get().asFile

    @Suppress("INACCESSIBLE_TYPE")
    style = compressed
}

tasks.getByName<KotlinWebpack>("jsBrowserProductionWebpack") {
    mainOutputFileName.set("output.js")
    sourceMaps = true
    outputDirectory.set(layout.buildDirectory.file("processedResources/jvm/main/static").get().asFile)
}

tasks.withType<AbstractCopyTask> {
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE
}

tasks.withType<AbstractArchiveTask> {
    isPreserveFileTimestamps = true
}

tasks.getByName<Jar>("jvmJar") {
    dependsOn(tasks.getByName("jsBrowserProductionWebpack"), tasks.getByName("compileSass"))
    val jsBrowserProductionWebpack = tasks.getByName<KotlinWebpack>("jsBrowserProductionWebpack")

    from(jsBrowserProductionWebpack.outputDirectory.get())
    listOf(jsBrowserProductionWebpack.mainOutputFileName.get(), jsBrowserProductionWebpack.mainOutputFileName.get() + ".map", "modules.js", "modules.js.map").forEach {
        from(File(jsBrowserProductionWebpack.outputDirectory.get().asFile, it))
    }
}

tasks.getByName<JavaExec>("run") {
    dependsOn(tasks.getByName<Jar>("jvmJar"))
    classpath(tasks.getByName<Jar>("jvmJar"))
}

tasks.getByName("jsBrowserDistribution") {
    dependsOn(tasks.getByName("jvmProcessResources"), tasks.getByName("compileSass"))
}

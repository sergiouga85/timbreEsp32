
        function tocarTimbre() {
            event.preventDefault();
            let xhr = new XMLHttpRequest();
            xhr.open("GET", "/toggle", true);
            xhr.send();
            xhr.onload = function () {
                if (xhr.status === 200) {
                    Swal.fire({
                      title: "Timbre Activado!",
                      icon: "success",
                      color: "write",
                    });
                } else {
                     Swal.fire({
                      title: "El Timbre no se activo",
                      icon: "error",
                      color: "write",
                     });
                }
            };

        }
        

        function guardarHorario() {
            // Evitar el comportamiento predeterminado del botón
            event.preventDefault();

            // Arrays para almacenar los datos
            const activadores = [];
            const inputTime = [];
            const dias = {
                lunes: [],
                martes: [],
                miercoles: [],
                jueves: [],
                viernes: [],
                sabado: [],
                domingo: []
            };
            const tiempoTimbre = [];

            // Llenar los arrays con los datos del formulario
            for (let i = 0; i < 16; i++) {
                activadores.push(Number(document.getElementById('activacion' + i).checked));
                inputTime.push(document.getElementById('time' + i).value);
                Object.keys(dias).forEach(dia => {
                    dias[dia].push(Number(document.getElementById(dia + 'Horario' + i).checked));
                });
                tiempoTimbre.push(document.getElementById('duracionTimbre' + i).value);
            }

            // Enviar los datos al servidor
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/guardar', true);
            xhr.setRequestHeader('Content-type', 'application/json');

            const dataToSend = {
                dataActivadores: activadores,
                dataTime: inputTime,
                dataDias: dias,
                dataTimbre: tiempoTimbre
            };

            xhr.send(JSON.stringify(dataToSend));
            xhr.onload = function () {
                if (xhr.status === 200) {
                    Swal.fire({
                      title: "Horario Guardado!",
                      icon: "success",
                      color: "write",
                    });
                } else {
                     Swal.fire({
                      title: "Error al guardar el horario",
                      icon: "error",
                      color: "write",
                     });
                }
            };
        }

        document.getElementById('btnGuardar').addEventListener('click', guardarHorario);



        window.onload = function () {
            obtenerDatosEEPROM();
        }

        window.onload = function () {
            obtenerDatosEEPROM();
        }



        function obtenerDatosEEPROM() {
            let xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    try {
                        const datos = JSON.parse(this.responseText);
                        console.log(datos);
                        actualizarDatosPagina(datos);
                    } catch (error) {
                        console.error("Error al parsear los datos:", error);
                    }
                }
            };
            xhr.open("GET", "/datos_eeprom", true);
            xhr.send();
        }

        function actualizarDatosPagina(datos) {

            for (let i = 0; i < 16; i++) {
                document.getElementById('activacion' + i).checked = Boolean(datos[i])
            }

            let Time = [];
            let hora = [];
            let minutos = [];
            let segundos = [];

            for (let j = 0; j < 16; j++) {

                if (datos[j + 16] < 10) {
                    hora.push("0" + datos[j + 16])
                } else {
                    hora.push(datos[j + 16])
                }

                if (datos[j + 32] < 10) {
                    minutos.push("0" + datos[j + 32])
                } else {
                    minutos.push(datos[j + 32])
                }

                if (datos[j + 48] < 10) {
                    for (let i = 0; i < 16; i++) {
                        segundos.push("0" + datos[i + 48])
                    }
                } else {
                    segundos.push(datos[j + 48])
                }

                Time.push(hora[j] + ":" + minutos[j] + ":" + segundos[j])

            }




            for (let i = 16; i < 32; i++) {
                document.getElementById('time' + (i - 16)).value = Time[i - 16]
            }

            for (let i = 64; i < 80; i++) {
                document.getElementById('lunesHorario' + (i - 64)).checked = Boolean(datos[i])
            }

            for (let i = 80; i < 96; i++) {
                document.getElementById('martesHorario' + (i - 80)).checked = Boolean(datos[i])
            }

            for (let i = 96; i < 112; i++) {
                document.getElementById('miercolesHorario' + (i - 96)).checked = Boolean(datos[i])
            }

            for (let i = 112; i < 128; i++) {
                document.getElementById('juevesHorario' + (i - 112)).checked = Boolean(datos[i])
            }

            for (let i = 128; i < 144; i++) {
                document.getElementById('viernesHorario' + (i - 128)).checked = Boolean(datos[i])
            }

            for (let i = 144; i < 160; i++) {
                document.getElementById('sabadoHorario' + (i - 144)).checked = Boolean(datos[i])
            }

            for (let i = 160; i < 176; i++) {
                document.getElementById('domingoHorario' + (i - 160)).checked = Boolean(datos[i])
            }

            for (let i = 176; i < 192; i++) {
                if (datos[i] < 10) {
                    document.getElementById('duracionTimbre' + (i - 176)).value = "0" + datos[i]
                } else {
                    document.getElementById('duracionTimbre' + (i - 176)).value = datos[i]
                }
            }



        }


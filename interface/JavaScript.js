function menubar() {
    const menu = document.getElementById('menuu');
    menu.classList.toggle('activo');
}

function showContent(seccion) {
    const pantallaCarga = document.getElementById('pantalla-carga');
    const secciones = ['apartado-inicio', 'apartado-pruebas', 'apartado-parametros'];

    pantallaCarga.style.display = 'flex';
    secciones.forEach(id => document.getElementById(id).style.display = 'none');
    document.getElementById('menuu').classList.remove('activo');

    setTimeout(() => {
        pantallaCarga.style.display = 'none';
        switch (seccion) {
            case 'inicio':
                document.getElementById('apartado-inicio').style.display = 'block';
                fetchAndDisplayParameters();
                break;
            case 'pruebas':
                document.getElementById('apartado-pruebas').style.display = 'block';
                break;
            case 'parametros':
                document.getElementById('apartado-parametros').style.display = 'block';
                break;
            case 'salir':
                fetch('/reiniciar', { method: 'POST' })
                    .then(() => setTimeout(() => location.reload(), 1000));
                break;
            default:
                console.warn(`Sección desconocida: ${seccion}`);
        }
    }, 1000);
}

function fetchAndDisplayParameters() {
    console.log("Solicitando parámetros actuales desde el servidor...");
    fetch("/get-parametros")
        .then(response => {
            if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
            return response.json();
        })
        .then(data => {
            const resumenId = document.getElementById('resumen-id');
            const resumenZona = document.getElementById('resumen-zona');
            const resumenTipo = document.getElementById('resumen-tipo-sensor');
            const resumenActualizacion = document.getElementById('resumen-actualizacion');

            const tipoSensorMap = {
                0: "0 - Gas LP",
                1: "1 - Humo",
                2: "2 - Movimiento",
                3: "3 - Puerta",
                4: "4 - Ventana",
                5: "5 - Cortina",
                6: "6 - Botón Físico/Pánico",
                7: "7 - Palanca",
                9: "9 - Test"
            };

            const tipoSensorText = tipoSensorMap[data.tipo] || `Tipo desconocido (${data.tipo})`;

            if (resumenId && resumenZona && resumenTipo && resumenActualizacion) {
                resumenId.textContent = data.id;
                resumenZona.textContent = data.zona;
                resumenTipo.textContent = tipoSensorText;
                resumenActualizacion.textContent = new Date().toLocaleString();
                console.log("Parámetros actualizados correctamente.");
            } else {
                console.error("No se encontraron todos los elementos de resumen.");
            }
        })
        .catch(error => {
            console.error("Error al obtener parámetros:", error);
            const resumenActualizacion = document.getElementById('resumen-actualizacion');
            if (resumenActualizacion) resumenActualizacion.textContent = "Error al cargar";
        });
}

function enviarLora(mensaje) {
    fetch('/enviar-lora', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mensaje })
    })
    .then(response => response.json())
    .then(data => {
        if (data.status) {
            alert(`Mensaje "${mensaje}" enviado correctamente por LoRa.`);
        } else if (data.error) {
            alert("Error: " + data.error);
        }
    })
    .catch(error => {
        alert("Error de comunicación con el servidor.");
        console.error(error);
    });
}

function mostrarPantallaLora(numero) {
    console.log("Solicitando mostrar pantalla en la tarjeta:", numero);
    fetch("/mostrar-pantalla", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ numero })
    })
    .then(response => response.json())
    .then(data => {
        if (data.status) {
            console.log("Pantalla mostrada correctamente:", data.status);
        } else if (data.error) {
            alert("Error al solicitar pantalla: " + data.error);
        } else {
            console.log("Respuesta desconocida al solicitar pantalla:", data);
        }
    })
    .catch(error => {
        alert("Error de comunicación con el servidor al solicitar pantalla.");
        console.error("Error en fetch:", error);
    });
}

document.addEventListener('DOMContentLoaded', () => {
    fetchAndDisplayParameters();

    const form = document.getElementById('form-parametros');
    if (form) {
        form.addEventListener('submit', function(e) {
            e.preventDefault();
            const idAlarma = document.getElementById('id-alarma').value;
            const zona = document.getElementById('zona').value;
            const tipoSensorSelect = document.getElementById('tipo-sensor');
            const tipoSensor = tipoSensorSelect.value;
            const tipoSensorText = tipoSensorSelect.options[tipoSensorSelect.selectedIndex].text;

            if (!/^\d{4}$/.test(idAlarma)) {
                alert('El ID de la alarma debe ser un número de 4 dígitos.');
                return;
            }

            const parametros = {
                "id-alarma": parseInt(idAlarma),
                "zona": parseInt(zona),
                "tipo-sensor": parseInt(tipoSensor)
            };

            console.log("Enviando parámetros:", parametros);
            fetch("/guardar-parametros", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(parametros)
            })
            .then(response => response.json())
            .then(data => {
                if (data.status) {
                    alert(data.status);
                    document.getElementById('resumen-id').textContent = idAlarma;
                    document.getElementById('resumen-zona').textContent = zona;
                    document.getElementById('resumen-tipo-sensor').textContent = tipoSensorText;
                    document.getElementById('resumen-actualizacion').textContent = new Date().toLocaleString();
                    console.log("Resumen actualizado.");
                } else if (data.error) {
                    alert("Error: " + data.error);
                } else {
                    alert("Respuesta desconocida del servidor.");
                }
            })
            .catch(error => {
                alert("Error de comunicación con el servidor.");
                console.error("Error en fetch:", error);
            });
        });
    }

    const btnEnviarRF = document.getElementById('btn-enviar-rf');
    if (btnEnviarRF) {
        btnEnviarRF.addEventListener('click', () => {
            console.log("Enviando señal RF de prueba...");
            fetch("/enviar-rf-prueba", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({})
            })
            .then(response => response.json())
            .then(data => {
                if (data.status) {
                    alert(data.status);
                } else if (data.error) {
                    alert("Error: " + data.error);
                } else {
                    alert("Respuesta desconocida del servidor.");
                }
            })
            .catch(error => {
                alert("Error de comunicación con el servidor al enviar RF.");
                console.error("Error en fetch:", error);
            });
        });
    }
});

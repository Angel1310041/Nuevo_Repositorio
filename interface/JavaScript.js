let parametrosActuales = {
    "id-alarma": null,
    "zona": null,
    "tipo-sensor": null
};

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

                // Actualizar parametrosActuales para enviar RF con los datos actuales
                parametrosActuales["id-alarma"] = data.id;
                parametrosActuales["zona"] = data.zona;
                parametrosActuales["tipo-sensor"] = data.tipo;
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
    const btnGuardar = document.getElementById('btnGuardar');
    const btnReiniciar = document.getElementById('btnReiniciar');
    const btnMostrarPantalla = document.getElementById('btnMostrarPantalla');
    const btnEnviarLora = document.getElementById('btnEnviarLora');
    const btnObtenerParametros = document.getElementById('btnObtenerParametros');
    const btnEnviarRFPrueba = document.getElementById('btnEnviarRFPrueba');

    // Guardar parámetros
    btnGuardar.addEventListener('click', () => {
        const idAlarma = parseInt(document.getElementById('idAlarma').value);
        const zona = parseInt(document.getElementById('zona').value);
        const tipoSensor = parseInt(document.getElementById('tipoSensor').value);

        const data = {
            "id-alarma": idAlarma,
            "zona": zona,
            "tipo-sensor": tipoSensor
        };

        fetch('/guardar-parametros', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        })
        .then(res => res.json())
        .then(json => alert(JSON.stringify(json)))
        .catch(err => alert('Error: ' + err));
    });

    // Reiniciar dispositivo
    btnReiniciar.addEventListener('click', () => {
        fetch('/reiniciar', { method: 'POST' })
        .then(() => alert('Reiniciando dispositivo...'))
        .catch(err => alert('Error: ' + err));
    });

    // Mostrar pantalla
    btnMostrarPantalla.addEventListener('click', () => {
        const numero = parseInt(document.getElementById('numeroPantalla').value);
        const data = { numero };

        fetch('/mostrar-pantalla', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        })
        .then(res => res.json())
        .then(json => alert(JSON.stringify(json)))
        .catch(err => alert('Error: ' + err));
    });

    // Enviar mensaje por Lora
    btnEnviarLora.addEventListener('click', () => {
        const mensaje = document.getElementById('mensajeLora').value;
        const data = { mensaje };

        fetch('/enviar-lora', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        })
        .then(res => res.json())
        .then(json => alert(JSON.stringify(json)))
        .catch(err => alert('Error: ' + err));
    });

    btnObtenerParametros.addEventListener('click', () => {
        fetch('/get-parametros')
        .then(res => res.json())
        .then(data => {
            document.getElementById('idAlarma').value = data.id;
            document.getElementById('zona').value = data.zona;
            document.getElementById('tipoSensor').value = data.tipo;
            alert('Parámetros cargados');
        })
        .catch(err => alert('Error: ' + err));
    });

   // Enviar señal de prueba RF con tipo 9
btnEnviarRFPrueba.addEventListener('click', () => {
    if (parametrosActuales["id-alarma"] === null ||
        parametrosActuales["zona"] === null ||
        parametrosActuales["tipo-sensor"] === null) {
        alert("Primero debes obtener y guardar los parámetros.");
        return;
    }

    const datosPrueba = {
        "id-alarma": parametrosActuales["id-alarma"],
        "zona": parametrosActuales["zona"],
        "tipo-sensor": 9 // Forzar tipo 9 para prueba
    };

    fetch('/enviar-rf', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(datosPrueba)
    })
    .then(res => res.json())
    .then(json => alert(`RF de prueba enviado: ${JSON.stringify(json)}`))
    .catch(err => alert('Error: ' + err));
});

});

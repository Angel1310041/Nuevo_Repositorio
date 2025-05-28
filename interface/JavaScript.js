function menubar() {
    var menu = document.getElementById('menuu');
    menu.classList.toggle('activo');
}
function showContent(seccion) {
    document.getElementById('pantalla-carga').style.display = 'flex';
    document.getElementById('apartado-inicio').style.display = 'none';
    document.getElementById('apartado-pruebas').style.display = 'none';
    document.getElementById('apartado-parametros').style.display = 'none';
    document.getElementById('menuu').classList.remove('activo');
    setTimeout(function() {
        document.getElementById('pantalla-carga').style.display = 'none';
        switch (seccion) {
            case 'inicio':
                document.getElementById('apartado-inicio').style.display = 'block';
                break;
            case 'pruebas':
                document.getElementById('apartado-pruebas').style.display = 'block';
                break;
            case 'parametros':
                document.getElementById('apartado-parametros').style.display = 'block';
                break;
            case 'salir':
                fetch('/reiniciar', { method: 'POST' })
                    .then(() => {
                        setTimeout(() => location.reload(), 1000);
                    });
                break;
        }
    }, 1000); 
}

document.addEventListener('DOMContentLoaded', function() {
    const form = document.getElementById('form-parametros');
    if (form) {
        form.addEventListener('submit', function(e) {
            e.preventDefault();
            const idAlarmaInput = document.getElementById('id-alarma');
            const zonaInput = document.getElementById('zona');
            const tipoSensorSelect = document.getElementById('tipo-sensor');
            const idAlarma = idAlarmaInput.value;
            const zona = zonaInput.value;
            const tipoSensor = tipoSensorSelect.value;

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
                headers: {
                    "Content-Type": "application/json" 
                },
                body: JSON.stringify(parametros) 
            })
            .then(response => response.json()) 
            .then(data => {
                if (data.status) {
                    alert(data.status); 
                    console.log("Respuesta del servidor:", data.status);
                } else if (data.error) {
                    alert("Error: " + data.error);
                    console.error("Error del servidor:", data.error);
                } else {
                     alert("Respuesta desconocida del servidor.");
                     console.log("Respuesta desconocida:", data);
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
        btnEnviarRF.addEventListener('click', function() {
            console.log("Enviando señal RF de prueba...");

            fetch("/enviar-rf-prueba", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json" 
                },
                body: JSON.stringify({}) 
            })
            .then(response => response.json()) 
            .then(data => {
                if (data.status) {
                    alert(data.status); 
                    console.log("Respuesta del servidor:", data.status);
                } else if (data.error) {
                    alert("Error: " + data.error); 
                    console.error("Error del servidor:", data.error);
                } else {
                     alert("Respuesta desconocida del servidor.");
                     console.log("Respuesta desconocida:", data);
                }
            })
            .catch(error => {
                alert("Error de comunicación con el servidor al enviar RF.");
                console.error("Error en fetch:", error);
            });
        });
    }

});

function enviarLora(mensaje) {
    fetch('/enviar-lora', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
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
    console.log("Pantalla seleccionada:", numero);
    enviarLora(String(numero));
}

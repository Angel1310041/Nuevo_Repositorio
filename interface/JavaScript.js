// Menú de navegación
function menubar() {
    var menu = document.getElementById('menuu');
    menu.classList.toggle('activo');
}

// Cambio de contenido por sección
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

// ... (código anterior) ...

// Inicialización al cargar el DOM
document.addEventListener('DOMContentLoaded', function() {
    const form = document.getElementById('form-parametros');
    if (form) {
        form.addEventListener('submit', function(e) {
            // Prevenir el envío por defecto del formulario
            e.preventDefault();

            const idAlarmaInput = document.getElementById('id-alarma');
            const zonaInput = document.getElementById('zona');
            const tipoSensorSelect = document.getElementById('tipo-sensor');

            const idAlarma = idAlarmaInput.value;
            const zona = zonaInput.value;
            const tipoSensor = tipoSensorSelect.value; // El valor del select ya es el número

            // Validación del ID de la alarma (la que ya tenías)
            if (!/^\d{4}$/.test(idAlarma)) {
                alert('El ID de la alarma debe ser un número de 4 dígitos.');
                return; // Detiene la ejecución si la validación falla
            }

            // Recoger los demás valores y convertirlos a número si es necesario
            // Los valores de input type="number" y select ya suelen ser strings que fetch puede enviar
            // El backend (ESP32) se encargará de parsearlos a int.

            // Crear el objeto con los datos del formulario
            const parametros = {
                "id-alarma": parseInt(idAlarma), // Convertir a número explícitamente
                "zona": parseInt(zona),         // Convertir a número explícitamente
                "tipo-sensor": parseInt(tipoSensor) // Convertir a número explícitamente
            };

            console.log("Enviando parámetros:", parametros); // Para depuración en el navegador

            // Enviar los datos al servidor usando fetch
            fetch("/guardar-parametros", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(parametros) // Enviar el objeto como JSON
            })
            .then(response => response.json())
            .then(data => {
                if (data.status) {
                    alert(data.status); // Mostrar mensaje de éxito
                    console.log("Respuesta del servidor:", data.status);
                } else if (data.error) {
                    alert("Error: " + data.error); // Mostrar mensaje de error
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
            enviarLora("56700001");
        });
    }
});



// Función para enviar datos por LoRa
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

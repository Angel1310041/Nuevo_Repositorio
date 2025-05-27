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

// Inicialización al cargar el DOM
document.addEventListener('DOMContentLoaded', function() {
    const form = document.getElementById('form-parametros');
    if (form) {
        form.addEventListener('submit', function(e) {
            const idAlarma = document.getElementById('id-alarma').value;
            if (!/^\d{4}$/.test(idAlarma)) {
                alert('El ID de la alarma debe ser un número de 4 dígitos.');
                e.preventDefault();
                return false;
            }
        });
    }

    document.getElementById("guardarBtn").addEventListener("click", () => {
    const parametro1 = document.getElementById("param1").value;
    const parametro2 = document.getElementById("param2").value;

    fetch("/guardar-parametros", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({ parametro1, parametro2 })
    })
    .then(res => res.json())
    .then(data => alert(data.status || data.error));
});

    // Botón para enviar la señal RF por LoRa
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

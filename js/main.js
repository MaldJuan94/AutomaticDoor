// Register Service Worker
if ('serviceWorker' in navigator) {
    window.addEventListener('load', function () {
    checkStorage();
     signIn();
        navigator.serviceWorker.register('./pwa-sw.js')
            .then(function (register) {
                console.log('PWA service worker ready');
                register.update();
            })
            .catch(function (error) {
                console.log('Register failed! Error:' + error);
            });

        // Check user internet status (online/offline)
        function updateOnlineStatus(event) {
            if (!navigator.onLine) {
                alert('Internet access is not possible!')
            }
        }

        window.addEventListener('online', updateOnlineStatus);
        window.addEventListener('offline', updateOnlineStatus);


        var animateButton = function(e) {

            opendoor();
          e.preventDefault;
          //reset animation
          e.target.classList.remove('animate');

          e.target.classList.add('animate');
          setTimeout(function(){
            e.target.classList.remove('animate');
          },700);
        };

        var bubblyButtons = document.getElementsByClassName("bubbly-button");

        for (var i = 0; i < bubblyButtons.length; i++) {
          bubblyButtons[i].addEventListener('click', animateButton, false);
        }

        async function signIn() {
            const url = 'https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key='+localStorage.getItem('key');;

            const requestBody = {
                email: localStorage.getItem('user'),
                password: localStorage.getItem('password'),
                returnSecureToken: true
            };

            try {
                const response = await fetch(url, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(requestBody)
                });
                const data = await response.json();

                if (response.ok) {
                    // Guardar el idToken y habilitar el botón
                    saveAuthData(data.idToken);
                    console.log('Autenticación exitosa');
                } else {
                    console.error('Error en la autenticación:', data.error.message);
                    setStatusButton(true);
                }
            } catch (error) {
                console.error('Error en la solicitud:', error);
                setStatusButton(true);
            }
        }


        async function opendoor() {
            const url = 'https://porterogirasol-default-rtdb.firebaseio.com/command/opendoor.json?auth='+getIdToken();

            const secondsSinceEpoch = Math.floor(Date.now() / 1000);
            const requestBody = "1000,"+secondsSinceEpoch;

            try {
                const response = await fetch(url, {
                    method: 'PUT',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(requestBody)
                });
                const data = await response.json();
                if (response.ok) {
                    vibrarDispositivo(1000);
                    showText('The door was opened');
                } else {
                    showText('Error opening the door');
                }
            } catch (error) {
                showText(' Something went really wrong');
                setStatusButton(true);
            }
        }

        function saveAuthData(idToken) {
            // Guardar el idToken en localStorage
            localStorage.setItem('idToken', idToken);
            setStatusButton(false);

        }

        function getIdToken() {
            return localStorage.getItem('idToken');
        }


        function showText(text) {
            const textElement = document.getElementById('fadeText');

            textElement.textContent = text;
            // Mostrar el texto
            textElement.classList.remove('hidden');
            textElement.classList.add('visible');

            // Después de 3 segundos, ocultar el texto de nuevo
            setTimeout(() => {
                textElement.classList.remove('visible');
                textElement.classList.add('hidden');
            }, 3000); // Cambiar el tiempo según lo que necesites
        }


        function vibrarDispositivo(duracion) {
              if ('vibrate' in navigator) {
                navigator.vibrate(duracion);
              } else {
                console.error('La API de Vibración no es compatible con este navegador.');
              }
          }

        function setStatusButton(disabled){
              // Habilitar el botón con id "opendoor"
                    const openDoorButton = document.getElementById('opendoor');
                    if (openDoorButton) {
                        openDoorButton.disabled = disabled; // Habilitar el botón
                        console.log('Botón "opendoor" habilitado');
                    }
        }


              // Función para mostrar el popup
                function showPopup() {
                    document.getElementById('popup').style.display = 'flex';
                }

                // Función para ocultar el popup
                function hidePopup() {
                    document.getElementById('popup').style.display = 'none';
                }

                // Comprobar si los datos ya están guardados en el localStorage
                function checkStorage() {
                    const user = localStorage.getItem('user');
                    const password = localStorage.getItem('password');
                    const key = localStorage.getItem('key');

                    // Si no están almacenados, muestra el popup
                    if (!user || !password || !key) {
                        showPopup();
                    }
                }

                // Guardar los datos en localStorage
                document.getElementById('saveBtn').addEventListener('click', () => {
                    const user = document.getElementById('user').value;
                    const password = document.getElementById('password').value;
                    const key = document.getElementById('key').value;

                    // Validar que los campos no estén vacíos
                    if (user && password && key) {
                        // Guardar los datos en localStorage
                        localStorage.setItem('user', user);
                        localStorage.setItem('password', password);
                        localStorage.setItem('key', key);

                        // Ocultar el popup después de guardar
                        hidePopup();
                        signIn();
                    } else {
                        alert('Please complete all fields');
                    }
                });

    });
}
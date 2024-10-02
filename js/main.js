// Register Service Worker
if ('serviceWorker' in navigator) {
    window.addEventListener('load', function () {

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
            const url = 'https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=AIzaSyBVfS68bzD0AREp8yUarOxFPcvev19LDu4';

            const requestBody = {
                email: "juanescorcia94@live.com",
                password: "mLemou12.",
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
            const url = 'https://porterogirasol-default-rtdb.firebaseio.com/commandv2/opendoor.json?auth='+getIdToken();

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


        function setStatusButton(disabled){
              // Habilitar el botón con id "opendoor"
                    const openDoorButton = document.getElementById('opendoor');
                    if (openDoorButton) {
                        openDoorButton.disabled = disabled; // Habilitar el botón
                        console.log('Botón "opendoor" habilitado');
                    }
        }

    });
}
import { useState, useEffect } from 'react';

function App() {
    const [nfcStatus, setNfcStatus] = useState(null);
    const [isAuthenticated, setIsAuthenticated] = useState(false);

    useEffect(() => {
        const interval = setInterval(async () => {
            try {
                const response = await fetch('http://localhost:5000/api/nfc/status');
                const data = await response.json();

                if (data.uid) {
                    const authResponse = await fetch('http://localhost:5000/api/nfc/check');
                    const authData = await authResponse.json();

                    setIsAuthenticated(authData.authorized);
                    setNfcStatus(data.uid);
                }
            } catch (error) {
                console.error('Erreur NFC:', error);
            }
        }, 500);

        return () => clearInterval(interval);
    }, []);

    return (
        <div className="app">
            {isAuthenticated ? (
                <div>
                    <h1>✅ Accès autorisé</h1>
                    <p>Badge: {nfcStatus}</p>
                </div>
            ) : (
                <div>
                    <h1>🔒 Veuillez badger</h1>
                </div>
            )}
        </div>
    );
}

export default App;

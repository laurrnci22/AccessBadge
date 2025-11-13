import { useState } from 'react';
import { Navbar } from './components/Navbar';
import { Home } from './pages/Home';
import { NfcAuth } from './pages/NfcAuth';
import { Register } from './pages/Register';

function App() {
    const [currentPage, setCurrentPage] = useState('home');

    const renderPage = () => {
        switch (currentPage) {
            case 'home':
                return <Home onNavigate={setCurrentPage} />;
            case 'nfc-auth':
                return <NfcAuth />;
            case 'register':
                return <Register />;
            default:
                return <Home onNavigate={setCurrentPage} />;
        }
    };

    return (
        <div className="min-h-screen bg-slate-950 relative overflow-hidden">
            {/* Animated background */}
            <div className="fixed inset-0 bg-[radial-gradient(ellipse_at_top_right,_var(--tw-gradient-stops))] from-blue-900/20 via-slate-950 to-slate-950"></div>
            <div className="fixed inset-0">
                <div className="absolute top-1/4 left-1/4 w-96 h-96 bg-blue-500/10 rounded-full blur-3xl animate-pulse"></div>
                <div className="absolute bottom-1/4 right-1/4 w-96 h-96 bg-cyan-500/10 rounded-full blur-3xl animate-pulse" style={{ animationDelay: '1s' }}></div>
            </div>

            <div className="relative z-10">
                <Navbar currentPage={currentPage} onNavigate={setCurrentPage} />
                <main className="container mx-auto px-4 py-8">
                    {renderPage()}
                </main>
            </div>

            {/* Grid overlay */}
            <div className="fixed inset-0 bg-[linear-gradient(rgba(59,130,246,0.03)_1px,transparent_1px),linear-gradient(90deg,rgba(59,130,246,0.03)_1px,transparent_1px)] bg-[size:50px_50px] pointer-events-none"></div>
        </div>
    );
}

export default App;

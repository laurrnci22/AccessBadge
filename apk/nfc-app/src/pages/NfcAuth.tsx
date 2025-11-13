import { useState, useEffect } from 'react';
import { CheckCircle2, Lock, Nfc, Activity } from 'lucide-react';
import { Loader } from '../components/Loader';

export function NfcAuth() {
  const [nfcStatus, setNfcStatus] = useState<string | null>(null);
  const [isAuthenticated, setIsAuthenticated] = useState(false);
  const [isLoading, setIsLoading] = useState(false);
  const [lastname, setLastname] = useState<string | null>(null);
  const [firstname, setFirstname] = useState<string | null>(null);
  const [password, setPassword] = useState<string | null>(null);

  useEffect(() => {
    const interval = setInterval(async () => {
      try {
        setIsLoading(true);
        const response = await fetch('http://localhost:5000/api/nfc/status');
        const data = await response.json();

        if (data.card_detected) {
          setLastname(data.info.nom);
          setFirstname(data.info.prenom);
          setPassword(data.info.password);
          setIsAuthenticated(true);
          setNfcStatus(data.info.uid);
        }
      } catch (error) {
        console.error('Erreur NFC:', error);
      } finally {
        setIsLoading(false);
      }
    }, 500);

    return () => clearInterval(interval);
  }, []);

  return (
      <div className="max-w-2xl mx-auto py-20">
        <div className="relative p-12 rounded-3xl bg-slate-900/40 backdrop-blur-xl border border-blue-500/20 overflow-hidden">
          <div className="absolute inset-0 bg-gradient-to-br from-blue-500/5 via-transparent to-cyan-500/5"></div>

          {isAuthenticated ? (
              <div className="relative space-y-8">
                {/* Success State */}
                <div className="text-center space-y-6">
                  <div className="relative inline-block">
                    <div className="absolute inset-0 bg-green-500 rounded-full blur-2xl opacity-30 animate-pulse"></div>
                    <div className="relative w-24 h-24 rounded-full bg-gradient-to-br from-green-500 to-emerald-500 flex items-center justify-center">
                      <CheckCircle2 className="w-12 h-12 text-white" strokeWidth={2.5} />
                    </div>
                  </div>

                  <div>
                    <h2 className="text-white text-3xl mb-2">Accès Autorisé</h2>
                    <p className="text-slate-400">Authentification réussie</p>
                  </div>
                </div>

                {/* Badge Info */}
                <div className="relative p-6 rounded-2xl bg-slate-800/40 backdrop-blur-xl border border-green-500/20 space-y-4">
                  <div className="flex items-center justify-between mb-4">
                    <span className="text-slate-400">Badge ID</span>
                    <div className="flex items-center gap-2">
                      <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></div>
                      <span className="text-green-400 text-sm">Active</span>
                    </div>
                  </div>

                  <div className="text-2xl font-mono text-white bg-slate-900/60 rounded-xl p-4 border border-green-500/20">
                    {nfcStatus}
                  </div>

                  {/* Personal Info */}
                  <div className="mt-4 grid grid-cols-1 md:grid-cols-3 gap-4">
                    <div className="p-4 rounded-xl bg-slate-700/50 backdrop-blur-md border border-green-500/20 text-center">
                      <span className="text-slate-400 text-sm block mb-1">Nom</span>
                      <span className="text-white font-semibold">{lastname}</span>
                    </div>
                    <div className="p-4 rounded-xl bg-slate-700/50 backdrop-blur-md border border-green-500/20 text-center">
                      <span className="text-slate-400 text-sm block mb-1">Prénom</span>
                      <span className="text-white font-semibold">{firstname}</span>
                    </div>
                    <div className="p-4 rounded-xl bg-slate-700/50 backdrop-blur-md border border-green-500/20 text-center">
                      <span className="text-slate-400 text-sm block mb-1">Password</span>
                      <span className="text-white font-mono">{password}</span>
                    </div>
                  </div>
                </div>

                {/* Status Grid */}
                <div className="grid grid-cols-3 gap-4">
                  {['Vérifié', 'Sécurisé', 'Connecté'].map((status, i) => (
                      <div key={i} className="text-center p-4 rounded-xl bg-slate-800/40 backdrop-blur-xl border border-blue-500/10">
                        <div className="w-2 h-2 bg-green-500 rounded-full mx-auto mb-2"></div>
                        <span className="text-slate-400 text-sm">{status}</span>
                      </div>
                  ))}
                </div>
              </div>
          ) : (
              <div className="relative space-y-8">
                {/* Waiting State */}
                <div className="text-center space-y-6">
                  <div className="relative inline-block">
                      <>
                        <div className="absolute inset-0 bg-blue-500 rounded-full blur-2xl opacity-20 animate-pulse"></div>
                        <div className="relative w-24 h-24 rounded-full bg-gradient-to-br from-blue-500 to-cyan-500 flex items-center justify-center">
                          <Nfc className="w-12 h-12 text-white" strokeWidth={2.5} />
                        </div>
                      </>
                  </div>
                  <div>
                    <h2 className="text-white text-3xl mb-2">Prêt à Scanner</h2>
                    <p className="text-slate-400">Approchez votre badge NFC</p>
                  </div>
                </div>

                {/* Scanning Area */}
                <div className="relative p-8 rounded-2xl bg-gradient-to-br from-blue-500/10 to-cyan-500/10 border border-blue-500/30">
                  <div className="flex items-center justify-center gap-3 text-blue-400 mb-6">
                    <Activity className="w-5 h-5 animate-pulse" />
                    <span>Détection en cours...</span>
                  </div>

                  <div className="flex justify-center gap-2">
                    {[0, 1, 2].map((i) => (
                        <div
                            key={i}
                            className="w-2 h-2 bg-blue-500 rounded-full animate-bounce"
                            style={{ animationDelay: `${i * 0.15}s` }}
                        ></div>
                    ))}
                  </div>
                </div>

                {/* Instructions */}
                <div className="flex items-start gap-3 p-4 rounded-xl bg-blue-500/5 border border-blue-500/20">
                  <div className="w-6 h-6 rounded-lg bg-blue-500/20 flex items-center justify-center flex-shrink-0 mt-0.5">
                    <span className="text-blue-400 text-sm">i</span>
                  </div>
                  <div className="text-sm text-slate-400">
                    Maintenez votre badge près du lecteur pendant 1-2 secondes pour une lecture optimale
                  </div>
                </div>
              </div>
          )}
        </div>

        {/* Connection Indicator */}
        <div className="mt-6 flex items-center justify-center gap-2 text-sm text-slate-500">
          <div className="w-1.5 h-1.5 bg-blue-500 rounded-full animate-pulse"></div>
          <span>Connected to localhost:5000</span>
        </div>
      </div>
  );
}

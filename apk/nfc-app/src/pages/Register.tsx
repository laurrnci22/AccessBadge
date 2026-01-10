import { useState } from 'react';
import { UserPlus, Nfc, CheckCircle2, Lock, User } from 'lucide-react'; 
import { Loader } from '../components/Loader';

export function Register() {
  const [name, setName] = useState('');
  const [prenom, setPrenom] = useState('');
  const [pass, setPass] = useState('');
  const [isScanning, setIsScanning] = useState(false);
  const [success, setSuccess] = useState(false);
  const [badgeId, setBadgeId] = useState('');

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setIsScanning(true);

    try {
      // Simulation de l'enregistrement du badge
      await new Promise((resolve) => setTimeout(resolve, 2000));
      
      // Appel API vers votre serveur Python/Node
      const response = await fetch('http://localhost:5000/api/nfc/register', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ 
          nom: name,        // 'name' dans React -> 'nom' pour Python
          prenom: prenom,   // 'prenom' -> 'prenom'
          password: pass    // 'pass' dans React -> 'password' pour Python
        })
      });

      if (response.ok) {
        setBadgeId('NFC-' + Math.random().toString(36).substr(2, 9).toUpperCase());
        setSuccess(true);
      }
    } catch (error) {
      console.error('Erreur lors de l\'enregistrement:', error);
    } finally {
      setIsScanning(false);
    }
  };

  const resetForm = () => {
    setName('');
    setPrenom('');
    setPass('');
    setSuccess(false);
    setBadgeId('');
  };

  return (
    <div className="max-w-2xl mx-auto py-20">
      <div className="relative p-12 rounded-3xl bg-slate-900/40 backdrop-blur-xl border border-blue-500/20 overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-br from-blue-500/5 via-transparent to-cyan-500/5"></div>
        
        {success ? (
          <div className="relative space-y-8">
            <div className="text-center space-y-6">
              <div className="relative inline-block">
                <div className="absolute inset-0 bg-green-500 rounded-full blur-2xl opacity-30 animate-pulse"></div>
                <div className="relative w-24 h-24 rounded-full bg-gradient-to-br from-green-500 to-emerald-500 flex items-center justify-center">
                  <CheckCircle2 className="w-12 h-12 text-white" strokeWidth={2.5} />
                </div>
              </div>
              <div>
                <h2 className="text-white text-3xl mb-2">Badge Enregistré</h2>
                <p className="text-slate-400">Bienvenue, {prenom} {name}</p>
              </div>
            </div>
            
            <div className="grid md:grid-cols-2 gap-4">
              <div className="p-5 rounded-2xl bg-slate-800/40 backdrop-blur-xl border border-blue-500/10">
                <div className="flex items-center gap-2 text-slate-500 mb-2 text-sm">
                  <User className="w-4 h-4" />
                  <span>Utilisateur</span>
                </div>
                <p className="text-white">{prenom} {name}</p>
              </div>
              
              <div className="p-5 rounded-2xl bg-slate-800/40 backdrop-blur-xl border border-blue-500/10">
                <div className="flex items-center gap-2 text-slate-500 mb-2 text-sm">
                  <Nfc className="w-4 h-4" />
                  <span>ID Badge</span>
                </div>
                <p className="text-white font-mono">{badgeId}</p>
              </div>
            </div>

            <button
              onClick={resetForm}
              className="w-full relative px-8 py-4 rounded-xl overflow-hidden group transition-all duration-300"
            >
              <div className="absolute inset-0 bg-gradient-to-r from-blue-600 to-cyan-600"></div>
              <span className="relative text-white font-medium">Enregistrer un autre badge</span>
            </button>
          </div>
        ) : (
          <div className="relative space-y-8">
            <div className="text-center space-y-4">
              <div className="relative inline-block">
                <div className="absolute inset-0 bg-blue-500 rounded-2xl blur-xl opacity-30"></div>
                <div className="relative w-16 h-16 rounded-2xl bg-gradient-to-br from-blue-500 to-cyan-500 flex items-center justify-center">
                  <UserPlus className="w-8 h-8 text-white" strokeWidth={2.5} />
                </div>
              </div>
              <h2 className="text-white text-3xl">Nouveau Badge</h2>
            </div>

            <form onSubmit={handleSubmit} className="space-y-5">
              <div className="grid md:grid-cols-2 gap-4">
                <div>
                  <label className="flex items-center gap-2 text-slate-400 mb-2 text-sm ml-1">
                    <User className="w-4 h-4" /> Nom
                  </label>
                  <input
                    type="text"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    required
                    className="w-full bg-slate-800/40 border border-blue-500/20 rounded-xl px-4 py-3 text-white focus:border-blue-500/50 outline-none"
                    placeholder="Dupont"
                  />
                </div>
                <div>
                  <label className="flex items-center gap-2 text-slate-400 mb-2 text-sm ml-1">
                    <User className="w-4 h-4" /> Prénom
                  </label>
                  <input
                    type="text"
                    value={prenom}
                    onChange={(e) => setPrenom(e.target.value)}
                    required
                    className="w-full bg-slate-800/40 border border-blue-500/20 rounded-xl px-4 py-3 text-white focus:border-blue-500/50 outline-none"
                    placeholder="Jean"
                  />
                </div>
              </div>

              <div>
                <label className="flex items-center gap-2 text-slate-400 mb-2 text-sm ml-1">
                  <Lock className="w-4 h-4" /> Mot de passe
                </label>
                <input
                  type="password"
                  value={pass}
                  onChange={(e) => setPass(e.target.value)}
                  required
                  className="w-full bg-slate-800/40 border border-blue-500/20 rounded-xl px-4 py-3 text-white focus:border-blue-500/50 outline-none"
                  placeholder="••••••••"
                />
              </div>

              {isScanning && (
                <div className="p-6 rounded-2xl bg-blue-500/10 border border-blue-500/30 text-center space-y-3">
                  <Loader />
                  <p className="text-blue-400 text-sm animate-pulse">Approchez le badge du lecteur...</p>
                </div>
              )}

              <button
                type="submit"
                disabled={isScanning}
                className="w-full relative px-8 py-4 rounded-xl overflow-hidden group transition-all disabled:opacity-50"
              >
                <div className="absolute inset-0 bg-gradient-to-r from-blue-600 to-cyan-600"></div>
                <div className="relative flex items-center justify-center gap-3 text-white font-bold">
                  {isScanning ? <span>Initialisation...</span> : <><Nfc className="w-5 h-5" /> Enregistrer</>}
                </div>
              </button>
            </form>
          </div>
        )}
      </div>
    </div>
  );
}
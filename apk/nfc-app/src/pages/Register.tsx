import { useState } from 'react';
import { UserPlus, Nfc, CheckCircle2, Mail, User } from 'lucide-react';
import { Loader } from '../components/Loader';

export function Register() {
  const [name, setName] = useState('');
  const [email, setEmail] = useState('');
  const [isScanning, setIsScanning] = useState(false);
  const [success, setSuccess] = useState(false);
  const [badgeId, setBadgeId] = useState('');

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setIsScanning(true);

    try {
      // Simulation de l'enregistrement du badge
      await new Promise((resolve) => setTimeout(resolve, 2000));
      
      // En production, vous feriez un appel API ici
      // const response = await fetch('http://localhost:5000/api/nfc/register', {
      //   method: 'POST',
      //   headers: { 'Content-Type': 'application/json' },
      //   body: JSON.stringify({ name, email })
      // });
      
      setBadgeId('NFC-' + Math.random().toString(36).substr(2, 9).toUpperCase());
      setSuccess(true);
    } catch (error) {
      console.error('Erreur lors de l\'enregistrement:', error);
    } finally {
      setIsScanning(false);
    }
  };

  const resetForm = () => {
    setName('');
    setEmail('');
    setSuccess(false);
    setBadgeId('');
  };

  return (
    <div className="max-w-2xl mx-auto py-20">
      <div className="relative p-12 rounded-3xl bg-slate-900/40 backdrop-blur-xl border border-blue-500/20 overflow-hidden">
        {/* Background effect */}
        <div className="absolute inset-0 bg-gradient-to-br from-blue-500/5 via-transparent to-cyan-500/5"></div>
        
        {success ? (
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
                <h2 className="text-white text-3xl mb-2">Badge Enregistré</h2>
                <p className="text-slate-400">Votre badge a été ajouté avec succès</p>
              </div>
            </div>
            
            {/* User Info */}
            <div className="grid md:grid-cols-2 gap-4">
              <div className="p-5 rounded-2xl bg-slate-800/40 backdrop-blur-xl border border-blue-500/10">
                <div className="flex items-center gap-2 text-slate-500 mb-2 text-sm">
                  <User className="w-4 h-4" />
                  <span>Nom</span>
                </div>
                <p className="text-white">{name}</p>
              </div>
              
              <div className="p-5 rounded-2xl bg-slate-800/40 backdrop-blur-xl border border-blue-500/10">
                <div className="flex items-center gap-2 text-slate-500 mb-2 text-sm">
                  <Mail className="w-4 h-4" />
                  <span>Email</span>
                </div>
                <p className="text-white truncate">{email}</p>
              </div>
            </div>
            
            {/* Badge ID */}
            <div className="p-6 rounded-2xl bg-gradient-to-br from-blue-500/10 to-cyan-500/10 border border-blue-500/30">
              <div className="flex items-center gap-2 text-slate-400 mb-4">
                <Nfc className="w-5 h-5" />
                <span>Badge ID</span>
              </div>
              <div className="text-2xl font-mono text-white bg-slate-900/60 rounded-xl p-4 border border-blue-500/20">
                {badgeId}
              </div>
            </div>

            <button
              onClick={resetForm}
              className="w-full relative px-8 py-4 rounded-xl overflow-hidden group transition-all duration-300"
            >
              <div className="absolute inset-0 bg-gradient-to-r from-blue-600 to-cyan-600"></div>
              <div className="absolute inset-0 bg-gradient-to-r from-blue-500 to-cyan-500 opacity-0 group-hover:opacity-100 transition-opacity"></div>
              <span className="relative text-white">Enregistrer un autre badge</span>
            </button>
          </div>
        ) : (
          <div className="relative space-y-8">
            {/* Header */}
            <div className="text-center space-y-4">
              <div className="relative inline-block">
                <div className="absolute inset-0 bg-blue-500 rounded-2xl blur-xl opacity-30"></div>
                <div className="relative w-16 h-16 rounded-2xl bg-gradient-to-br from-blue-500 to-cyan-500 flex items-center justify-center">
                  <UserPlus className="w-8 h-8 text-white" strokeWidth={2.5} />
                </div>
              </div>
              <div>
                <h2 className="text-white text-3xl mb-2">Nouveau Badge</h2>
                <p className="text-slate-400">Complétez vos informations</p>
              </div>
            </div>

            {/* Form */}
            <form onSubmit={handleSubmit} className="space-y-6">
              <div>
                <label htmlFor="name" className="flex items-center gap-2 text-slate-400 mb-3">
                  <User className="w-4 h-4" />
                  <span>Nom complet</span>
                </label>
                <input
                  type="text"
                  id="name"
                  value={name}
                  onChange={(e) => setName(e.target.value)}
                  required
                  className="w-full bg-slate-800/40 backdrop-blur-xl border border-blue-500/20 rounded-xl px-5 py-4 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500/50 transition-all duration-200"
                  placeholder="Jean Dupont"
                />
              </div>

              <div>
                <label htmlFor="email" className="flex items-center gap-2 text-slate-400 mb-3">
                  <Mail className="w-4 h-4" />
                  <span>Adresse email</span>
                </label>
                <input
                  type="email"
                  id="email"
                  value={email}
                  onChange={(e) => setEmail(e.target.value)}
                  required
                  className="w-full bg-slate-800/40 backdrop-blur-xl border border-blue-500/20 rounded-xl px-5 py-4 text-white placeholder-slate-500 focus:outline-none focus:border-blue-500/50 transition-all duration-200"
                  placeholder="jean.dupont@example.com"
                />
              </div>

              {isScanning && (
                <div className="p-8 rounded-2xl bg-gradient-to-br from-blue-500/10 to-cyan-500/10 border border-blue-500/30 text-center space-y-4">
                  <Loader />
                  <div>
                    <p className="text-blue-400 mb-1">Scan en cours...</p>
                    <p className="text-slate-500 text-sm">Approchez votre badge du lecteur</p>
                  </div>
                </div>
              )}

              <button
                type="submit"
                disabled={isScanning}
                className="w-full relative px-8 py-4 rounded-xl overflow-hidden group transition-all duration-300 disabled:opacity-50 disabled:cursor-not-allowed"
              >
                <div className="absolute inset-0 bg-gradient-to-r from-blue-600 to-cyan-600"></div>
                <div className="absolute inset-0 bg-gradient-to-r from-blue-500 to-cyan-500 opacity-0 group-hover:opacity-100 transition-opacity"></div>
                <div className="relative flex items-center justify-center gap-3 text-white">
                  {isScanning ? (
                    <>
                      <Loader />
                      <span>Enregistrement...</span>
                    </>
                  ) : (
                    <>
                      <Nfc className="w-5 h-5" />
                      <span>Enregistrer le badge</span>
                    </>
                  )}
                </div>
              </button>
            </form>
          </div>
        )}
      </div>

      {/* Steps */}
      <div className="grid md:grid-cols-2 gap-4 mt-6">
        <div className="flex items-start gap-3 p-4 rounded-xl bg-slate-900/40 backdrop-blur-xl border border-blue-500/10">
          <div className="w-6 h-6 rounded-lg bg-blue-500/20 flex items-center justify-center flex-shrink-0 text-blue-400 text-sm">1</div>
          <div className="text-sm text-slate-400">Remplissez vos informations personnelles</div>
        </div>
        <div className="flex items-start gap-3 p-4 rounded-xl bg-slate-900/40 backdrop-blur-xl border border-blue-500/10">
          <div className="w-6 h-6 rounded-lg bg-cyan-500/20 flex items-center justify-center flex-shrink-0 text-cyan-400 text-sm">2</div>
          <div className="text-sm text-slate-400">Approchez votre badge du lecteur NFC</div>
        </div>
      </div>
    </div>
  );
}

import { Nfc, Shield, Zap, ChevronRight, Radio } from 'lucide-react';

interface HomeProps {
  onNavigate: (page: string) => void;
}

export function Home({ onNavigate }: HomeProps) {
  const features = [
    {
      icon: Nfc,
      title: 'NFC Authentication',
      description: 'Technologie sans contact ultra-rapide',
      color: 'from-blue-500 to-cyan-500',
    },
    {
      icon: Shield,
      title: 'Security First',
      description: 'Encryption de niveau militaire',
      color: 'from-cyan-500 to-blue-400',
    },
    {
      icon: Zap,
      title: 'Lightning Fast',
      description: 'Accès instantané en 0.5 secondes',
      color: 'from-blue-400 to-sky-500',
    },
  ];

  return (
    <div className="max-w-7xl mx-auto">
      {/* Hero Section */}
      <div className="relative py-20 md:py-32">
        <div className="text-center space-y-8">
          <div className="inline-flex items-center gap-2 px-4 py-2 rounded-full bg-blue-500/10 border border-blue-500/20 text-blue-400 backdrop-blur-xl">
            <Radio className="w-4 h-4" />
            <span>Next-Gen Authentication</span>
          </div>
          
          <h1 className="text-white text-5xl md:text-7xl tracking-tight max-w-4xl mx-auto leading-tight">
            Authentification
            <span className="block bg-gradient-to-r from-blue-400 via-cyan-400 to-blue-500 bg-clip-text text-transparent">
              Sans Friction
            </span>
          </h1>
          
          <p className="text-slate-400 text-lg md:text-xl max-w-2xl mx-auto leading-relaxed">
            Badgez et accédez. Notre système NFC révolutionnaire 
            vous identifie en un instant.
          </p>

          <div className="flex gap-4 justify-center flex-wrap pt-4">
            <button
              onClick={() => onNavigate('nfc-auth')}
              className="group relative px-8 py-4 rounded-xl overflow-hidden transition-all duration-300 hover:scale-105"
            >
              <div className="absolute inset-0 bg-gradient-to-r from-blue-600 to-cyan-600"></div>
              <div className="absolute inset-0 bg-gradient-to-r from-blue-500 to-cyan-500 opacity-0 group-hover:opacity-100 transition-opacity"></div>
              <div className="relative flex items-center gap-2 text-white">
                <span>Démarrer</span>
                <ChevronRight className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
              </div>
            </button>
            
            <button
              onClick={() => onNavigate('register')}
              className="relative px-8 py-4 rounded-xl bg-slate-800/40 backdrop-blur-xl border border-blue-500/20 text-white hover:bg-slate-800/60 hover:border-blue-500/40 transition-all duration-300"
            >
              Enregistrer un badge
            </button>
          </div>
        </div>
      </div>

      {/* Features Grid */}
      <div className="grid md:grid-cols-3 gap-6 py-12">
        {features.map((feature, index) => {
          const Icon = feature.icon;
          return (
            <div
              key={index}
              className="group relative p-8 rounded-2xl bg-slate-900/40 backdrop-blur-xl border border-blue-500/10 hover:border-blue-500/30 transition-all duration-300 overflow-hidden"
            >
              <div className="absolute inset-0 bg-gradient-to-br from-blue-500/5 to-transparent opacity-0 group-hover:opacity-100 transition-opacity"></div>
              
              <div className="relative space-y-4">
                <div className={`w-12 h-12 rounded-xl bg-gradient-to-br ${feature.color} p-0.5`}>
                  <div className="w-full h-full bg-slate-900 rounded-xl flex items-center justify-center">
                    <Icon className="w-6 h-6 text-blue-400" />
                  </div>
                </div>
                
                <h3 className="text-white text-xl">{feature.title}</h3>
                <p className="text-slate-400">{feature.description}</p>
              </div>

              <div className="absolute top-0 right-0 w-32 h-32 bg-blue-500/5 rounded-full blur-3xl"></div>
            </div>
          );
        })}
      </div>

      {/* Stats Bar */}
      <div className="relative mt-16 p-8 rounded-2xl bg-slate-900/40 backdrop-blur-xl border border-blue-500/20 overflow-hidden">
        <div className="absolute inset-0 bg-gradient-to-r from-blue-500/10 via-transparent to-cyan-500/10"></div>
        
        <div className="relative grid md:grid-cols-3 gap-8">
          <div className="text-center">
            <div className="text-4xl md:text-5xl bg-gradient-to-r from-blue-400 to-cyan-400 bg-clip-text text-transparent mb-2">
              500ms
            </div>
            <div className="text-slate-400">Temps de réponse</div>
          </div>
          <div className="text-center border-x border-blue-500/10">
            <div className="text-4xl md:text-5xl bg-gradient-to-r from-blue-400 to-cyan-400 bg-clip-text text-transparent mb-2">
              AES-256
            </div>
            <div className="text-slate-400">Encryption</div>
          </div>
          <div className="text-center">
            <div className="text-4xl md:text-5xl bg-gradient-to-r from-blue-400 to-cyan-400 bg-clip-text text-transparent mb-2">
              99.9%
            </div>
            <div className="text-slate-400">Uptime</div>
          </div>
        </div>
      </div>
    </div>
  );
}

import { Nfc, Home, UserPlus } from 'lucide-react';

interface NavbarProps {
  currentPage: string;
  onNavigate: (page: string) => void;
}

export function Navbar({ currentPage, onNavigate }: NavbarProps) {
  const navItems = [
    { id: 'home', label: 'Accueil', icon: Home },
    { id: 'nfc-auth', label: 'Authentification', icon: Nfc },
    { id: 'register', label: 'Inscription', icon: UserPlus },
  ];

  return (
    <nav className="relative border-b border-blue-500/10">
      <div className="absolute inset-0 bg-slate-900/40 backdrop-blur-2xl"></div>
      
      <div className="relative container mx-auto px-4">
        <div className="flex items-center justify-between h-20">
          <div className="flex items-center gap-4">
            <div className="relative group">
              <div className="absolute inset-0 bg-blue-500 rounded-xl blur-lg opacity-50 group-hover:opacity-75 transition-opacity"></div>
              <div className="relative w-12 h-12 bg-gradient-to-br from-blue-500 to-cyan-500 rounded-xl flex items-center justify-center">
                <Nfc className="w-6 h-6 text-white" strokeWidth={2.5} />
              </div>
            </div>
            <div>
              <div className="text-white tracking-tight">NFC System</div>
              <div className="text-xs text-blue-400">Secure Access</div>
            </div>
          </div>

          <div className="flex gap-2">
            {navItems.map((item) => {
              const Icon = item.icon;
              const isActive = currentPage === item.id;
              
              return (
                <button
                  key={item.id}
                  onClick={() => onNavigate(item.id)}
                  className={`relative group flex items-center gap-2 px-5 py-2.5 rounded-xl transition-all duration-300 ${
                    isActive
                      ? 'text-white'
                      : 'text-slate-400 hover:text-white'
                  }`}
                >
                  {isActive && (
                    <div className="absolute inset-0 bg-blue-500/20 backdrop-blur-xl rounded-xl border border-blue-500/30"></div>
                  )}
                  <Icon className="w-4 h-4 relative z-10" />
                  <span className="hidden sm:inline relative z-10">{item.label}</span>
                </button>
              );
            })}
          </div>
        </div>
      </div>
    </nav>
  );
}

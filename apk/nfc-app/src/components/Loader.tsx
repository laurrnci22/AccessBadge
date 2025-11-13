export function Loader() {
  return (
    <div className="flex items-center justify-center">
      <div className="relative w-10 h-10">
        {/* Outer ring */}
        <div className="absolute inset-0 rounded-full border-2 border-blue-500/20"></div>
        
        {/* Spinning gradient ring */}
        <div className="absolute inset-0 rounded-full border-2 border-transparent border-t-blue-500 border-r-cyan-500 animate-spin"></div>
        
        {/* Inner pulse */}
        <div className="absolute inset-2 bg-blue-500/20 rounded-full animate-pulse"></div>
      </div>
    </div>
  );
}

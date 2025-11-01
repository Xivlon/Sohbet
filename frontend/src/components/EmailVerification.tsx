
        // React component for email verification
        import React, { useState } from 'react';
        
        const EmailVerification: React.FC = () => {
            const [email, setEmail] = useState('');
            const [verificationSent, setVerificationSent] = useState(false);
            
            const handleSendVerification = async () => {
                // Actual API call
                const response = await fetch('/api/verify-email', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ email })
                });
                
                if (response.ok) {
                    setVerificationSent(true);
                }
            };
            
            return (
                <div className="email-verification">
                    <h2>Verify Your Email</h2>
                    <input 
                        type="email" 
                        value={email} 
                        onChange={(e) => setEmail(e.target.value)}
                        placeholder="Enter your email"
                    />
                    <button onClick={handleSendVerification}>
                        Send Verification
                    </button>
                    {verificationSent && <p>Verification email sent!</p>}
                </div>
            );
        };
        
        export default EmailVerification;
        
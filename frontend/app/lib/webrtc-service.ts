/**
 * WebRTC Service for Voice and Video Communication
 * Manages peer-to-peer connections, media streams, and audio/video state
 */
'use client';

import { websocketService } from './websocket-service';

export interface VoiceParticipant {
  userId: number;
  username: string;
  university: string;
  isMuted: boolean;
  isVideoEnabled: boolean;
  isSpeaking: boolean;
  audioLevel: number;
  stream?: MediaStream;
}

export interface WebRTCConfig {
  iceServers: RTCIceServer[];
}

// Default STUN and TURN servers for NAT traversal and relay
// STUN servers help discover public IP addresses for NAT traversal
// TURN servers provide relay when direct P2P connection fails
// Using multiple TURN servers as fallbacks for reliability
// Optimized to use 4 servers (under the 5-server warning threshold)
const DEFAULT_ICE_SERVERS: RTCIceServer[] = [
  // Google's public STUN server (primary)
  { urls: 'stun:stun.l.google.com:19302' },
  { urls: 'stun:stun1.l.google.com:19302' },

  // Primary TURN: Twilio's STUN server (more reliable than free TURN)
  { urls: 'stun:global.stun.twilio.com:3478' },

  // Fallback: OpenRelay TURN servers (free but may be unreliable)
  // Cloudflare STUN server (reliable backup)
  { urls: 'stun:stun.cloudflare.com:3478' },
  // Twilio TURN server (reliable, multiple transports)
  {
    urls: [
      'turn:global.turn.twilio.com:3478?transport=udp',
      'turn:global.turn.twilio.com:3478?transport=tcp',
      'turn:global.turn.twilio.com:443?transport=tcp'
    ],
    username: 'f4b4035eaa76f4a55de5f4351567653ee4ff6fa97b50b6b334fcc1be9c27212d',
    credential: 'w1uxM55V9yVoqyVFjt+mxDBV0F87AUCemaYVQGxsPLw=',
  },
  // Metered TURN server (backup, multiple transports)
  {
    urls: [
      'turn:a.relay.metered.ca:80',
      'turn:a.relay.metered.ca:80?transport=tcp',
      'turn:a.relay.metered.ca:443',
      'turn:a.relay.metered.ca:443?transport=tcp'
    ],
    username: 'e244935f05c942d47a93c5b4',
    credential: 'RYzUexu5W/Tb0gSy',
  },
  {
    urls: 'turn:openrelay.metered.ca:443?transport=tcp',
    username: 'openrelayproject',
    credential: 'openrelayproject',
  },
];

class WebRTCService {
  private localStream: MediaStream | null = null;
  private peerConnections: Map<number, RTCPeerConnection> = new Map();
  private participants: Map<number, VoiceParticipant> = new Map();
  private currentChannelId: number | null = null;
  private currentUserId: number | null = null;
  private audioContext: AudioContext | null = null;
  private audioAnalyzers: Map<number, AnalyserNode> = new Map();
  private audioGainNodes: Map<number, GainNode> = new Map(); // For volume control
  private audioAnimationFrames: Map<number, number> = new Map(); // Store animation frame IDs
  private isMuted: boolean = false;
  private isVideoEnabled: boolean = false;
  private isDeafened: boolean = false;
  private participantVolumes: Map<number, number> = new Map(); // Individual participant volumes (0-1)
  private pendingIceCandidates: Map<number, RTCIceCandidateInit[]> = new Map(); // Queue ICE candidates until remote description is set
  private iceRestartAttempts: Map<number, number> = new Map(); // Track ICE restart attempts per user
  private readonly MAX_ICE_RESTART_ATTEMPTS = 3; // Maximum ICE restart attempts before giving up

  // Callbacks for UI updates
  private onParticipantUpdateCallback: ((participants: VoiceParticipant[]) => void) | null = null;
  private onRemoteStreamCallback: ((userId: number, stream: MediaStream) => void) | null = null;
  private onConnectionQualityCallback: ((quality: 'good' | 'medium' | 'poor') => void) | null = null;

  constructor(private config: WebRTCConfig = { iceServers: DEFAULT_ICE_SERVERS }) {
    this.setupWebSocketHandlers();
  }

  /**
   * Setup WebSocket handlers for voice signaling
   */
  private setupWebSocketHandlers() {
    // Handle new user joining the channel
    websocketService.on('voice:user-joined', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      const { user_id, username, university, channel_id } = payload;

      if (channel_id !== this.currentChannelId) return;

      console.log('User joined voice channel:', username);

      // Add to participants
      this.participants.set(user_id, {
        userId: user_id,
        username,
        university: university || '',
        isMuted: false,
        isVideoEnabled: false,
        isSpeaking: false,
        audioLevel: 0,
      });

      // Initiate WebRTC connection if this is not us
      if (user_id !== this.currentUserId) {
        // Use polite/impolite pattern: if we're "impolite" (higher user ID),
        // add a small delay to reduce chance of offer collision
        const isPolite = (this.currentUserId ?? 0) < user_id;
        const delay = isPolite ? 0 : 100; // Impolite side waits 100ms

        setTimeout(() => {
          // Only create offer if still in channel and user is still a participant
          if (this.currentChannelId === channel_id && this.participants.has(user_id)) {
            this.createOffer(user_id);
          }
        }, delay);
      }

      this.notifyParticipantUpdate();
    });

    // Handle user leaving the channel
    websocketService.on('voice:user-left', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      const { user_id, channel_id } = payload;

      if (channel_id !== this.currentChannelId) return;

      console.log('User left voice channel:', user_id);

      // Close peer connection
      this.closePeerConnection(user_id);

      // Remove from participants
      this.participants.delete(user_id);
      this.notifyParticipantUpdate();
    });

    // Handle receiving list of existing participants
    websocketService.on('voice:participants', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      const { participants } = payload;

      participants.forEach((p: any, index: number) => {
        this.participants.set(p.user_id, {
          userId: p.user_id,
          username: p.username,
          university: p.university || '',
          isMuted: false,
          isVideoEnabled: false,
          isSpeaking: false,
          audioLevel: 0,
        });

        // Create offers to all existing participants (excluding ourselves)
        if (p.user_id !== this.currentUserId) {
          // Use polite/impolite pattern with staggered delays for multiple participants
          const isPolite = (this.currentUserId ?? 0) < p.user_id;
          // Add extra delay based on index to stagger multiple offers
          const baseDelay = isPolite ? 0 : 100;
          const delay = baseDelay + (index * 50); // Stagger by 50ms per participant

          setTimeout(() => {
            // Only create offer if still in channel and user is still a participant
            if (this.currentChannelId !== null && this.participants.has(p.user_id)) {
              this.createOffer(p.user_id);
            }
          }, delay);
        }
      });

      this.notifyParticipantUpdate();
    });

    // Handle WebRTC offer
    websocketService.on('voice:offer', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      this.handleOffer(payload);
    });

    // Handle WebRTC answer
    websocketService.on('voice:answer', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      this.handleAnswer(payload);
    });

    // Handle ICE candidate
    websocketService.on('voice:ice-candidate', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      this.handleIceCandidate(payload);
    });

    // Handle user mute status
    websocketService.on('voice:user-muted', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      const { user_id, muted } = payload;

      const participant = this.participants.get(user_id);
      if (participant) {
        participant.isMuted = muted;
        this.notifyParticipantUpdate();
      }
    });

    // Handle user video toggle
    websocketService.on('voice:user-video-toggled', (message) => {
      // Parse payload if it's a string
      const payload = typeof message.payload === 'string'
        ? JSON.parse(message.payload)
        : message.payload as any;
      const { user_id, video_enabled } = payload;

      const participant = this.participants.get(user_id);
      if (participant) {
        participant.isVideoEnabled = video_enabled;
        this.notifyParticipantUpdate();
      }
    });
  }

  /**
   * Join a voice channel
   */
  async joinChannel(channelId: number, userId: number, audioOnly: boolean = true): Promise<void> {
    // If already in a channel, leave it first to prevent resource leaks
    if (this.currentChannelId !== null) {
      console.log('Already in a channel, leaving before joining new one');
      this.leaveChannel();
    }

    this.currentChannelId = channelId;
    this.currentUserId = userId;
    this.isVideoEnabled = false;

    // Clear any existing participants from previous channels
    this.participants.clear();

    try {
      // Get local media stream
      this.localStream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: true,
          noiseSuppression: true,
          autoGainControl: true,
        },
        video: audioOnly ? false : {
          width: { ideal: 1280 },
          height: { ideal: 720 },
        },
      });

      console.log('Got local media stream');

      // Setup audio analyzer for speaking detection
      this.setupAudioAnalyzer();

      // Notify server that we're joining
      websocketService.send('voice:join', { channel_id: channelId });

    } catch (error) {
      console.error('Failed to get user media:', error);
      throw new Error('Failed to access microphone/camera. Please grant permissions.');
    }
  }

  /**
   * Leave the current voice channel
   */
  leaveChannel() {
    if (!this.currentChannelId) return;

    // Notify server
    websocketService.send('voice:leave', { channel_id: this.currentChannelId });

    // Close all peer connections
    this.peerConnections.forEach((pc, userId) => {
      this.closePeerConnection(userId);
    });
    this.peerConnections.clear();

    // Stop local stream
    if (this.localStream) {
      this.localStream.getTracks().forEach(track => track.stop());
      this.localStream = null;
    }

    // Cancel all animation frames
    this.audioAnimationFrames.forEach((animationId) => {
      cancelAnimationFrame(animationId);
    });
    this.audioAnimationFrames.clear();

    // Cleanup audio context
    if (this.audioContext) {
      this.audioContext.close();
      this.audioContext = null;
    }
    this.audioAnalyzers.clear();

    // Clear pending ICE candidates
    this.pendingIceCandidates.clear();

    // Clear ICE restart tracking
    this.iceRestartAttempts.clear();

    // Clear all connection failure timeouts
    this.connectionFailureTimeouts.forEach((timeout) => {
      clearTimeout(timeout);
    });
    this.connectionFailureTimeouts.clear();

    // Clear state
    this.participants.clear();
    this.currentChannelId = null;
    this.currentUserId = null;
    this.notifyParticipantUpdate();
  }

  /**
   * Get local media stream
   */
  getLocalStream(): MediaStream | null {
    return this.localStream;
  }

  /**
   * Toggle mute status
   */
  toggleMute(): boolean {
    if (!this.localStream) return false;

    this.isMuted = !this.isMuted;

    this.localStream.getAudioTracks().forEach(track => {
      track.enabled = !this.isMuted;
    });

    // Notify other users
    if (this.currentChannelId) {
      websocketService.send('voice:mute', {
        channel_id: this.currentChannelId,
        muted: this.isMuted,
      });
    }

    return this.isMuted;
  }

  /**
   * Toggle video
   */
  async toggleVideo(): Promise<boolean> {
    if (!this.localStream) return false;

    const videoTracks = this.localStream.getVideoTracks();

    // If no video tracks exist, request camera permission and add video tracks
    if (videoTracks.length === 0) {
      try {
        console.log('No video tracks found, requesting camera permission...');

        // Request video stream
        const videoStream = await navigator.mediaDevices.getUserMedia({
          video: {
            width: { ideal: 1280 },
            height: { ideal: 720 },
          },
        });

        // Add video tracks to the local stream
        videoStream.getVideoTracks().forEach(track => {
          this.localStream!.addTrack(track);

          // Add the new video track to all existing peer connections
          this.peerConnections.forEach((pc, userId) => {
            const sender = pc.getSenders().find(s => s.track?.kind === 'video');
            if (!sender) {
              // No video sender exists, add one
              pc.addTrack(track, this.localStream!);
              console.log(`Added video track to peer connection with user ${userId}`);
            } else {
              // Replace existing video sender's track
              sender.replaceTrack(track);
              console.log(`Replaced video track in peer connection with user ${userId}`);
            }
          });
        });

        this.isVideoEnabled = true;
        console.log('Camera enabled successfully');
      } catch (error) {
        console.error('Failed to get camera permission:', error);
        throw new Error('Failed to access camera. Please grant camera permissions.');
      }
    } else {
      // Video tracks exist, just toggle them
      this.isVideoEnabled = !this.isVideoEnabled;
      videoTracks.forEach(track => {
        track.enabled = this.isVideoEnabled;
      });
    }

    // Notify other users
    if (this.currentChannelId) {
      websocketService.send('voice:video-toggle', {
        channel_id: this.currentChannelId,
        video_enabled: this.isVideoEnabled,
      });
    }

    return this.isVideoEnabled;
  }

  /**
   * Get current mute status
   */
  isMutedStatus(): boolean {
    return this.isMuted;
  }

  /**
   * Get current video status
   */
  isVideoEnabledStatus(): boolean {
    return this.isVideoEnabled;
  }

  /**
   * Toggle deafen status (mute all incoming audio)
   */
  toggleDeafen(): boolean {
    this.isDeafened = !this.isDeafened;

    // Mute/unmute all remote audio elements
    this.peerConnections.forEach((pc, userId) => {
      const remoteStreams = pc.getReceivers()
        .map(receiver => receiver.track)
        .filter(track => track.kind === 'audio');

      remoteStreams.forEach(track => {
        track.enabled = !this.isDeafened;
      });
    });

    // If deafening, also mute microphone
    if (this.isDeafened && !this.isMuted) {
      this.toggleMute();
    }

    return this.isDeafened;
  }

  /**
   * Get current deafen status
   */
  isDeafenedStatus(): boolean {
    return this.isDeafened;
  }

  /**
   * Set volume for a specific participant (0-1)
   */
  setParticipantVolume(userId: number, volume: number) {
    const clampedVolume = Math.max(0, Math.min(1, volume));
    this.participantVolumes.set(userId, clampedVolume);

    // Apply volume to the GainNode if it exists
    const gainNode = this.audioGainNodes.get(userId);
    if (gainNode) {
      gainNode.gain.value = clampedVolume;
      console.log(`Set volume for user ${userId} to ${clampedVolume}`);
    } else {
      console.warn(`No gain node found for user ${userId}, volume will be applied when stream connects`);
    }

    this.notifyParticipantUpdate();
  }

  /**
   * Get volume for a specific participant
   */
  getParticipantVolume(userId: number): number {
    return this.participantVolumes.get(userId) || 1.0;
  }

  /**
   * Setup audio analyzer for speaking detection
   */
  private setupAudioAnalyzer() {
    if (!this.localStream) return;

    // Close existing audio context if it exists to prevent resource leak
    if (this.audioContext && this.audioContext.state !== 'closed') {
      this.audioContext.close().catch(err =>
        console.error('Error closing old audio context:', err)
      );
    }

    this.audioContext = new AudioContext();
    const source = this.audioContext.createMediaStreamSource(this.localStream);
    const analyzer = this.audioContext.createAnalyser();
    analyzer.fftSize = 256;
    source.connect(analyzer);

    // Store analyzer for this user
    if (this.currentUserId) {
      this.audioAnalyzers.set(this.currentUserId, analyzer);

      // Start monitoring audio levels
      this.monitorAudioLevel(this.currentUserId, analyzer);
    }
  }

  /**
   * Monitor audio level for speaking detection
   */
  private monitorAudioLevel(userId: number, analyzer: AnalyserNode) {
    const bufferLength = analyzer.frequencyBinCount;
    const dataArray = new Uint8Array(bufferLength);

    const checkAudioLevel = () => {
      // Schedule next frame and store ID immediately for proper cleanup
      const animationId = requestAnimationFrame(checkAudioLevel);
      this.audioAnimationFrames.set(userId, animationId);

      // Stop if analyzer removed
      if (!this.audioAnalyzers.has(userId)) {
        // Cancel the animation frame we just scheduled
        cancelAnimationFrame(animationId);
        this.audioAnimationFrames.delete(userId);
        return;
      }

      analyzer.getByteFrequencyData(dataArray);

      // Calculate average volume
      const average = dataArray.reduce((a, b) => a + b) / bufferLength;
      const normalizedLevel = average / 255;

      // Update speaking status (threshold: 0.01)
      const isSpeaking = !this.isMuted && normalizedLevel > 0.01;

      // Update local participant
      const participant = this.participants.get(userId);
      if (participant) {
        participant.audioLevel = normalizedLevel;
        participant.isSpeaking = isSpeaking;
        this.notifyParticipantUpdate();
      }
    };

    checkAudioLevel();
  }

  /**
   * Create WebRTC peer connection and send offer
   */
  private async createOffer(targetUserId: number) {
    if (!this.localStream || !this.currentChannelId) return;

    const pc = this.createPeerConnection(targetUserId);

    try {
      // Check signaling state before creating offer
      // Only create offer if we're in a stable state or if we need to renegotiate
      if (pc.signalingState !== 'stable' && pc.signalingState !== 'have-local-offer') {
        console.log(`Skipping offer creation for user ${targetUserId} - signaling state is ${pc.signalingState}`);
        return;
      }

      // If we already have a local offer pending, don't create another one
      if (pc.signalingState === 'have-local-offer') {
        console.log(`Already have pending offer for user ${targetUserId}, skipping`);
        return;
      }

      // Add local tracks to peer connection only if not already added
      this.localStream.getTracks().forEach(track => {
        // Check if this track is already added to a sender
        const existingSender = pc.getSenders().find(sender => sender.track === track);
        if (!existingSender) {
          pc.addTrack(track, this.localStream!);
        }
      });

      // Create and send offer
      const offer = await pc.createOffer();
      await pc.setLocalDescription(offer);

      console.log('Sending offer to user:', targetUserId);

      websocketService.send('voice:offer', {
        channel_id: this.currentChannelId,
        target_user_id: targetUserId,
        from_user_id: this.currentUserId,
        offer: {
          type: offer.type,
          sdp: offer.sdp,
        },
      });
    } catch (error) {
      console.error('Error creating offer:', error);
    }
  }

  /**
   * Handle incoming WebRTC offer
   */
  private async handleOffer(payload: any) {
    const { from_user_id, offer } = payload;

    if (!this.localStream || !this.currentChannelId) return;

    const pc = this.createPeerConnection(from_user_id);

    try {
      // Handle offer collision (both sides send offers simultaneously)
      // Use polite/impolite pattern: lower user ID is "polite" and rolls back
      const isPolite = (this.currentUserId ?? 0) < from_user_id;
      const offerCollision = pc.signalingState !== 'stable';

      console.log(`Handling offer from user ${from_user_id}, signaling state: ${pc.signalingState}, isPolite: ${isPolite}, collision: ${offerCollision}`);

      if (offerCollision) {
        if (!isPolite) {
          // Impolite side: ignore the incoming offer and keep our own
          console.log(`Ignoring offer from user ${from_user_id} due to glare (we are impolite, keeping our offer)`);
          return;
        } else {
          // Polite side: rollback our offer and accept the incoming one
          console.log(`Rolling back our offer due to collision with user ${from_user_id} (we are polite)`);

          // Only rollback if we're in have-local-offer state
          if (pc.signalingState === 'have-local-offer') {
            await pc.setLocalDescription({ type: 'rollback' });
          }
        }
      }

      // Add local tracks only if not already added
      this.localStream.getTracks().forEach(track => {
        // Check if this track is already added to a sender
        const existingSender = pc.getSenders().find(sender => sender.track === track);
        if (!existingSender) {
          pc.addTrack(track, this.localStream!);
        }
      });

      // Set remote description
      await pc.setRemoteDescription(new RTCSessionDescription(offer));
      console.log(`Set remote description from offer (user ${from_user_id})`);

      // Process any queued ICE candidates now that remote description is set
      await this.processQueuedIceCandidates(from_user_id, pc);

      // Create and send answer
      const answer = await pc.createAnswer();
      await pc.setLocalDescription(answer);

      console.log('Sending answer to user:', from_user_id);

      websocketService.send('voice:answer', {
        channel_id: this.currentChannelId,
        target_user_id: from_user_id,
        from_user_id: this.currentUserId,
        answer: {
          type: answer.type,
          sdp: answer.sdp,
        },
      });
    } catch (error) {
      console.error('Error handling offer:', error);
      // If we fail to handle the offer, try to recover by closing and recreating the peer connection
      console.log(`Attempting to recover from offer handling error with user ${from_user_id}`);
      this.closePeerConnection(from_user_id);
    }
  }

  /**
   * Handle incoming WebRTC answer
   */
  private async handleAnswer(payload: any) {
    const { from_user_id, answer } = payload;

    const pc = this.peerConnections.get(from_user_id);
    if (!pc) {
      console.error('No peer connection found for user:', from_user_id);
      return;
    }

    try {
      await pc.setRemoteDescription(new RTCSessionDescription(answer));
      console.log('Set remote description from answer');

      // Process any queued ICE candidates now that remote description is set
      await this.processQueuedIceCandidates(from_user_id, pc);
    } catch (error) {
      console.error('Error handling answer:', error);
    }
  }

  /**
   * Handle incoming ICE candidate
   */
  private async handleIceCandidate(payload: any) {
    const { from_user_id, candidate } = payload;

    const pc = this.peerConnections.get(from_user_id);
    if (!pc) {
      console.error('No peer connection found for user:', from_user_id);
      return;
    }

    try {
      if (candidate) {
        // Check if remote description is set
        if (!pc.remoteDescription) {
          // Queue the candidate until remote description is set
          if (!this.pendingIceCandidates.has(from_user_id)) {
            this.pendingIceCandidates.set(from_user_id, []);
          }
          this.pendingIceCandidates.get(from_user_id)!.push(candidate);
          console.log(`Queued ICE candidate from user ${from_user_id} (waiting for remote description)`);
          return;
        }

        await pc.addIceCandidate(new RTCIceCandidate(candidate));
        console.log(`Added ICE candidate from user ${from_user_id}`);
      }
    } catch (error) {
      console.error(`Error adding ICE candidate from user ${from_user_id}:`, error);
      // Don't close connection on ICE candidate failure - connection might still work
      // Just log the error and let ICE gathering continue
    }
  }

  /**
   * Process queued ICE candidates after remote description is set
   */
  private async processQueuedIceCandidates(userId: number, pc: RTCPeerConnection) {
    const queuedCandidates = this.pendingIceCandidates.get(userId);
    if (queuedCandidates && queuedCandidates.length > 0) {
      console.log(`Processing ${queuedCandidates.length} queued ICE candidates for user ${userId}`);

      for (const candidate of queuedCandidates) {
        try {
          await pc.addIceCandidate(new RTCIceCandidate(candidate));
        } catch (error) {
          console.error(`Error adding queued ICE candidate for user ${userId}:`, error);
        }
      }

      // Clear the queue
      this.pendingIceCandidates.delete(userId);
    }
  }

  /**
   * Create RTCPeerConnection for a specific user
   */
  private createPeerConnection(userId: number): RTCPeerConnection {
    // Check if connection already exists
    if (this.peerConnections.has(userId)) {
      return this.peerConnections.get(userId)!;
    }

    const pc = new RTCPeerConnection({
      iceServers: this.config.iceServers,
      // Use relay as fallback to force TURN usage if direct connection fails
      iceTransportPolicy: 'all', // Try all: host, srflx, relay
      // Bundle all media on single transport for better NAT traversal
      bundlePolicy: 'max-bundle',
      // Use unified plan for better browser compatibility
      sdpSemantics: 'unified-plan' as RTCSdpSemantics,
    });

    // Handle ICE candidates
    pc.onicecandidate = (event) => {
      if (event.candidate && this.currentChannelId) {
        websocketService.send('voice:ice-candidate', {
          channel_id: this.currentChannelId,
          target_user_id: userId,
          from_user_id: this.currentUserId,
          candidate: event.candidate.toJSON(),
        });
      }
    };

    // Handle incoming remote stream
    pc.ontrack = (event) => {
      console.log('Received remote track from user:', userId);

      const stream = event.streams[0];

      // Update participant stream
      const participant = this.participants.get(userId);
      if (participant) {
        participant.stream = stream;
        this.notifyParticipantUpdate();
      }

      // Notify callback
      if (this.onRemoteStreamCallback) {
        this.onRemoteStreamCallback(userId, stream);
      }

      // Setup audio analysis for remote stream
      this.setupRemoteAudioAnalyzer(userId, stream);
    };

    // Handle connection state changes
    pc.onconnectionstatechange = () => {
      console.log(`Connection state with user ${userId}:`, pc.connectionState);

      if (pc.connectionState === 'failed') {
        console.error(`Connection failed with user ${userId} - will attempt recovery via ICE restart`);
        // Don't immediately close - ICE restart handler will attempt recovery
        this.updateConnectionQuality();
      } else if (pc.connectionState === 'closed') {
        console.error(`Connection closed with user ${userId}`);
        this.closePeerConnection(userId);
        this.updateConnectionQuality();
      } else if (pc.connectionState === 'connected') {
        console.log(`Successfully connected to user ${userId}`);
        this.updateConnectionQuality();
        // Start monitoring connection quality
        this.monitorConnectionQuality(userId, pc);
      }
    };

    // Handle ICE connection state changes for better debugging
    pc.oniceconnectionstatechange = () => {
      console.log(`ICE connection state with user ${userId}:`, pc.iceConnectionState);

      if (pc.iceConnectionState === 'failed') {
        console.error(`ICE connection failed with user ${userId}. Attempting recovery...`);

        // Check restart attempts
        const restartAttempts = this.iceRestartAttempts.get(userId) || 0;
        const MAX_RESTART_ATTEMPTS = 3;

        if (restartAttempts < MAX_RESTART_ATTEMPTS) {
          // Attempt to restart ICE
          this.iceRestartAttempts.set(userId, restartAttempts + 1);
          console.log(`ICE restart attempt ${restartAttempts + 1}/${MAX_RESTART_ATTEMPTS} for user ${userId}`);
          this.restartIce(userId);
        } else {
          // Max attempts reached, close and recreate the connection
          console.error(`Max ICE restart attempts reached for user ${userId}. Closing connection.`);
          this.closePeerConnection(userId);

          // Try to re-establish connection after a delay (only if we're still in the channel)
          if (this.currentChannelId !== null) {
            setTimeout(() => {
              if (this.currentChannelId !== null && this.participants.has(userId)) {
                console.log(`Attempting to re-establish connection with user ${userId}`);
                this.iceRestartAttempts.delete(userId); // Reset restart attempts
                this.createOffer(userId);
              }
            }, 2000);
          }
        }
      } else if (pc.iceConnectionState === 'disconnected') {
        console.warn(`ICE connection disconnected with user ${userId}. Waiting for automatic reconnection...`);

        // Clear any existing timeout
        const existingTimeout = this.connectionFailureTimeouts.get(userId);
        if (existingTimeout) {
          clearTimeout(existingTimeout);
        }

        // Set a timeout to try ICE restart if it doesn't reconnect automatically
        const timeout = setTimeout(() => {
          if (pc.iceConnectionState === 'disconnected') {
            console.log(`ICE still disconnected after timeout, attempting restart for user ${userId}`);
            this.restartIce(userId);
          }
        }, 5000); // Wait 5 seconds before attempting restart

        this.connectionFailureTimeouts.set(userId, timeout);
      } else if (pc.iceConnectionState === 'connected' || pc.iceConnectionState === 'completed') {
        console.log(`ICE connection established with user ${userId}`);
        // Reset restart attempts counter on successful connection
        this.iceRestartAttempts.delete(userId);

        // Reset restart attempts on successful connection
        this.iceRestartAttempts.delete(userId);

        // Clear any pending failure timeouts
        const existingTimeout = this.connectionFailureTimeouts.get(userId);
        if (existingTimeout) {
          clearTimeout(existingTimeout);
          this.connectionFailureTimeouts.delete(userId);
        }
      }
    };

    // Handle ICE gathering state changes
    pc.onicegatheringstatechange = () => {
      console.log(`ICE gathering state with user ${userId}:`, pc.iceGatheringState);
    };

    // Handle negotiation needed (when tracks are added/removed dynamically)
    pc.onnegotiationneeded = async () => {
      try {
        console.log(`Negotiation needed with user ${userId}, creating new offer`);

        // Only create offer if we're in a stable state or have local description
        if (pc.signalingState !== 'stable' && pc.signalingState !== 'have-local-offer') {
          console.log(`Skipping negotiation - signaling state is ${pc.signalingState}`);
          return;
        }

        // Create and send new offer
        const offer = await pc.createOffer();
        await pc.setLocalDescription(offer);

        if (this.currentChannelId) {
          websocketService.send('voice:offer', {
            channel_id: this.currentChannelId,
            target_user_id: userId,
            from_user_id: this.currentUserId,
            offer: {
              type: offer.type,
              sdp: offer.sdp,
            },
          });
          console.log(`Sent renegotiation offer to user ${userId}`);
        }
      } catch (error) {
        console.error(`Error during negotiation with user ${userId}:`, error);
      }
    };

    this.peerConnections.set(userId, pc);
    return pc;
  }

  /**
   * Setup audio analyzer for remote stream
   */
  private setupRemoteAudioAnalyzer(userId: number, stream: MediaStream) {
    // Clean up existing analyzer and gain node for this user to prevent duplicates
    const existingGainNode = this.audioGainNodes.get(userId);
    if (existingGainNode) {
      existingGainNode.disconnect();
      this.audioGainNodes.delete(userId);
      console.log(`Cleaned up existing audio nodes for user ${userId}`);
    }

    // Cancel existing animation frame for this user
    const existingAnimationId = this.audioAnimationFrames.get(userId);
    if (existingAnimationId !== undefined) {
      cancelAnimationFrame(existingAnimationId);
      this.audioAnimationFrames.delete(userId);
    }

    if (!this.audioContext || this.audioContext.state === 'closed') {
      this.audioContext = new AudioContext();
    }

    // Resume audio context if suspended (required by some browsers)
    if (this.audioContext.state === 'suspended') {
      this.audioContext.resume().catch(err =>
        console.error('Failed to resume audio context:', err)
      );
    }

    const source = this.audioContext.createMediaStreamSource(stream);
    const analyzer = this.audioContext.createAnalyser();
    analyzer.fftSize = 256;

    // Create gain node for volume control
    const gainNode = this.audioContext.createGain();

    // Apply stored volume or default to 1.0
    const storedVolume = this.participantVolumes.get(userId) ?? 1.0;
    gainNode.gain.value = storedVolume;

    // CRITICAL FIX: Connect audio pipeline properly
    // source → analyzer (for speaker detection)
    // source → gainNode → destination (for audio output with volume control)
    source.connect(analyzer);
    source.connect(gainNode);
    gainNode.connect(this.audioContext.destination);

    this.audioAnalyzers.set(userId, analyzer);
    this.audioGainNodes.set(userId, gainNode);
    this.monitorAudioLevel(userId, analyzer);

    console.log(`Setup audio for user ${userId} with volume ${storedVolume}`);
  }

  /**
   * Attempt to restart ICE connection
   */
  private async restartIce(userId: number) {
    const pc = this.peerConnections.get(userId);
    if (!pc || !this.currentChannelId) return;

    // Check retry limit
    const attempts = this.iceRestartAttempts.get(userId) || 0;
    if (attempts >= this.MAX_ICE_RESTART_ATTEMPTS) {
      console.error(`Max ICE restart attempts (${this.MAX_ICE_RESTART_ATTEMPTS}) reached for user ${userId}. Giving up.`);
      console.error(`This likely means TURN server is unavailable or both users are behind strict NAT/firewall.`);
      console.error(`Consider setting up a dedicated TURN server for production use.`);
      // Close the failed connection
      this.closePeerConnection(userId);
      return;
    }

    try {
      console.log(`Restarting ICE connection with user ${userId} (attempt ${attempts + 1}/${this.MAX_ICE_RESTART_ATTEMPTS})`);
      this.iceRestartAttempts.set(userId, attempts + 1);

      // Create new offer with iceRestart flag
      const offer = await pc.createOffer({ iceRestart: true });
      await pc.setLocalDescription(offer);

      websocketService.send('voice:offer', {
        channel_id: this.currentChannelId,
        target_user_id: userId,
        from_user_id: this.currentUserId,
        offer: {
          type: offer.type,
          sdp: offer.sdp,
        },
      });
    } catch (error) {
      console.error('Error restarting ICE:', error);
    }
  }

  /**
   * Close peer connection
   */
  private closePeerConnection(userId: number) {
    const pc = this.peerConnections.get(userId);
    if (pc) {
      pc.close();
      this.peerConnections.delete(userId);
    }

    // Cancel animation frame for this user
    const animationId = this.audioAnimationFrames.get(userId);
    if (animationId !== undefined) {
      cancelAnimationFrame(animationId);
      this.audioAnimationFrames.delete(userId);
    }

    // Clean up audio nodes
    const gainNode = this.audioGainNodes.get(userId);
    if (gainNode) {
      gainNode.disconnect();
      this.audioGainNodes.delete(userId);
    }

    this.audioAnalyzers.delete(userId);
    this.participantVolumes.delete(userId);

    // Clean up queued ICE candidates
    this.pendingIceCandidates.delete(userId);

    // Clean up ICE restart attempts counter
    this.iceRestartAttempts.delete(userId);
    // Clean up ICE restart tracking
    this.iceRestartAttempts.delete(userId);

    // Clean up connection failure timeouts
    const timeout = this.connectionFailureTimeouts.get(userId);
    if (timeout) {
      clearTimeout(timeout);
      this.connectionFailureTimeouts.delete(userId);
    }
  }

  /**
   * Register callback for participant updates
   */
  onParticipantUpdate(callback: (participants: VoiceParticipant[]) => void) {
    this.onParticipantUpdateCallback = callback;
  }

  /**
   * Register callback for remote streams
   */
  onRemoteStream(callback: (userId: number, stream: MediaStream) => void) {
    this.onRemoteStreamCallback = callback;
  }

  /**
   * Register callback for connection quality updates
   */
  onConnectionQuality(callback: (quality: 'good' | 'medium' | 'poor') => void) {
    this.onConnectionQualityCallback = callback;
  }

  /**
   * Get current participants
   */
  getParticipants(): VoiceParticipant[] {
    return Array.from(this.participants.values());
  }

  /**
   * Notify participant update
   */
  private notifyParticipantUpdate() {
    if (this.onParticipantUpdateCallback) {
      this.onParticipantUpdateCallback(this.getParticipants());
    }
  }

  /**
   * Monitor connection quality using WebRTC stats
   */
  private async monitorConnectionQuality(userId: number, pc: RTCPeerConnection) {
    const checkQuality = async () => {
      const stats = await pc.getStats();
      let packetsLost = 0;
      let packetsReceived = 0;
      let jitter = 0;

      stats.forEach((report) => {
        if (report.type === 'inbound-rtp' && report.kind === 'audio') {
          packetsLost = report.packetsLost || 0;
          packetsReceived = report.packetsReceived || 0;
          jitter = report.jitter || 0;
        }
      });

      // Don't update if not connected anymore
      if (!this.peerConnections.has(userId)) {
        return;
      }

      this.updateConnectionQuality();

      // Check again in 2 seconds
      setTimeout(() => checkQuality(), 2000);
    };

    checkQuality();
  }

  /**
   * Update overall connection quality based on all peer connections
   */
  private async updateConnectionQuality() {
    if (this.peerConnections.size === 0) {
      if (this.onConnectionQualityCallback) {
        this.onConnectionQualityCallback('good');
      }
      return;
    }

    let totalConnections = 0;
    let goodConnections = 0;
    let mediumConnections = 0;
    let poorConnections = 0;

    for (const [userId, pc] of this.peerConnections) {
      totalConnections++;
      const state = pc.connectionState;

      if (state === 'connected') {
        try {
          const stats = await pc.getStats();
          let packetsLost = 0;
          let packetsReceived = 0;

          stats.forEach((report) => {
            if (report.type === 'inbound-rtp' && report.kind === 'audio') {
              packetsLost = report.packetsLost || 0;
              packetsReceived = report.packetsReceived || 0;
            }
          });

          const lossRate = packetsReceived > 0 ? packetsLost / packetsReceived : 0;

          if (lossRate < 0.02) {
            goodConnections++;
          } else if (lossRate < 0.05) {
            mediumConnections++;
          } else {
            poorConnections++;
          }
        } catch (err) {
          mediumConnections++;
        }
      } else if (state === 'connecting' || state === 'new') {
        mediumConnections++;
      } else {
        poorConnections++;
      }
    }

    // Determine overall quality
    let quality: 'good' | 'medium' | 'poor' = 'good';
    if (poorConnections > 0 || goodConnections < totalConnections / 2) {
      quality = 'poor';
    } else if (mediumConnections > 0) {
      quality = 'medium';
    }

    if (this.onConnectionQualityCallback) {
      this.onConnectionQualityCallback(quality);
    }
  }
}

// Export singleton instance
export const webrtcService = new WebRTCService();

export default webrtcService;

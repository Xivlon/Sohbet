variable "aws_region" {
  description = "Primary AWS region"
  type        = string
  default     = "us-east-1"
}

variable "environment" {
  description = "Environment name (dev, staging, production)"
  type        = string
  default     = "production"
}

variable "project_name" {
  description = "Project name for resource naming"
  type        = string
  default     = "sohbet-voice"
}

# Network Configuration
variable "vpc_cidr" {
  description = "CIDR block for VPC"
  type        = string
  default     = "10.0.0.0/16"
}

variable "availability_zones" {
  description = "Availability zones to use"
  type        = list(string)
  default     = ["us-east-1a", "us-east-1b", "us-east-1c"]
}

# Backend Configuration
variable "backend_instance_count" {
  description = "Number of backend instances"
  type        = number
  default     = 3
}

variable "backend_instance_type" {
  description = "EC2 instance type for backend"
  type        = string
  default     = "t3.large" # 2 vCPU, 8GB RAM
}

variable "backend_ami_id" {
  description = "AMI ID for backend (Ubuntu 22.04 LTS)"
  type        = string
  default     = "ami-0c7217cdde317cfec" # Ubuntu 22.04 us-east-1
}

# Redis Configuration
variable "redis_node_type" {
  description = "ElastiCache node type"
  type        = string
  default     = "cache.t3.medium" # 2 vCPU, 3.09GB RAM
}

variable "redis_num_cache_nodes" {
  description = "Number of Redis cache nodes"
  type        = number
  default     = 3
}

# LiveKit Configuration
variable "livekit_node_count" {
  description = "Number of LiveKit SFU nodes"
  type        = number
  default     = 3
}

variable "livekit_instance_type" {
  description = "EC2 instance type for LiveKit"
  type        = string
  default     = "c5.2xlarge" # 8 vCPU, 16GB RAM
}

# TURN Server Configuration
variable "turn_regions" {
  description = "AWS regions for TURN servers"
  type        = list(string)
  default     = ["us-east-1", "eu-west-1", "ap-northeast-1"]
}

variable "turn_instance_type" {
  description = "EC2 instance type for TURN servers"
  type        = string
  default     = "t3.large" # 2 vCPU, 8GB RAM
}

# Load Balancer Configuration
variable "lb_type" {
  description = "Load balancer type (application or network)"
  type        = string
  default     = "application"
}

variable "certificate_arn" {
  description = "ACM certificate ARN for HTTPS"
  type        = string
  default     = ""
}

# Monitoring Configuration
variable "enable_monitoring" {
  description = "Enable Prometheus and Grafana"
  type        = bool
  default     = true
}

variable "monitoring_instance_type" {
  description = "EC2 instance type for monitoring"
  type        = string
  default     = "t3.large"
}

# Secrets
variable "livekit_api_key" {
  description = "LiveKit API key"
  type        = string
  sensitive   = true
}

variable "livekit_api_secret" {
  description = "LiveKit API secret"
  type        = string
  sensitive   = true
}

variable "turn_secret_key" {
  description = "TURN server secret key"
  type        = string
  sensitive   = true
}

variable "redis_auth_token" {
  description = "Redis authentication token"
  type        = string
  sensitive   = true
}

# Tags
variable "additional_tags" {
  description = "Additional tags for resources"
  type        = map(string)
  default     = {}
}

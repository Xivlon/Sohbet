# Network Outputs
output "vpc_id" {
  description = "VPC ID"
  value       = module.vpc.vpc_id
}

output "private_subnet_ids" {
  description = "Private subnet IDs"
  value       = module.vpc.private_subnet_ids
}

output "public_subnet_ids" {
  description = "Public subnet IDs"
  value       = module.vpc.public_subnet_ids
}

# Backend Outputs
output "backend_instance_ids" {
  description = "Backend EC2 instance IDs"
  value       = module.backend.instance_ids
}

output "backend_private_ips" {
  description = "Backend private IP addresses"
  value       = module.backend.private_ips
}

# Redis Outputs
output "redis_endpoint" {
  description = "Redis cluster endpoint"
  value       = module.redis.redis_endpoint
  sensitive   = true
}

output "redis_port" {
  description = "Redis port"
  value       = module.redis.redis_port
}

# LiveKit Outputs
output "livekit_instance_ids" {
  description = "LiveKit instance IDs"
  value       = module.livekit.instance_ids
}

output "livekit_urls" {
  description = "LiveKit WebSocket URLs"
  value       = module.livekit.websocket_urls
}

# TURN Server Outputs
output "turn_servers" {
  description = "TURN server addresses by region"
  value = {
    us_east = module.turn_us.public_ip
    eu_west = module.turn_eu.public_ip
    asia    = module.turn_asia.public_ip
  }
}

# Load Balancer Outputs
output "alb_dns_name" {
  description = "Application Load Balancer DNS name"
  value       = module.loadbalancer.alb_dns_name
}

output "alb_zone_id" {
  description = "ALB hosted zone ID (for Route53)"
  value       = module.loadbalancer.alb_zone_id
}

# Monitoring Outputs
output "prometheus_url" {
  description = "Prometheus URL"
  value       = var.enable_monitoring ? "http://${module.monitoring[0].prometheus_public_ip}:9090" : null
}

output "grafana_url" {
  description = "Grafana URL"
  value       = var.enable_monitoring ? "http://${module.monitoring[0].grafana_public_ip}:3000" : null
}

# Connection Information
output "ssh_bastion_host" {
  description = "Bastion host for SSH access"
  value       = module.vpc.bastion_public_ip
}

output "websocket_endpoint" {
  description = "WebSocket endpoint for clients"
  value       = "wss://${module.loadbalancer.alb_dns_name}/ws"
}

output "api_endpoint" {
  description = "REST API endpoint"
  value       = "https://${module.loadbalancer.alb_dns_name}/api"
}

# Summary
output "deployment_summary" {
  description = "Deployment summary"
  value = {
    environment        = var.environment
    region            = var.aws_region
    backend_count     = var.backend_instance_count
    redis_nodes       = var.redis_num_cache_nodes
    livekit_nodes     = var.livekit_node_count
    turn_regions      = var.turn_regions
    monitoring_enabled = var.enable_monitoring
  }
}

# Estimated Monthly Cost
output "estimated_monthly_cost_usd" {
  description = "Estimated monthly cost in USD"
  value = {
    backend    = var.backend_instance_count * 50
    redis      = var.redis_num_cache_nodes * 30
    livekit    = var.livekit_node_count * 150
    turn       = length(var.turn_regions) * 500
    monitoring = var.enable_monitoring ? 50 : 0
    total      = (var.backend_instance_count * 50) +
                 (var.redis_num_cache_nodes * 30) +
                 (var.livekit_node_count * 150) +
                 (length(var.turn_regions) * 500) +
                 (var.enable_monitoring ? 50 : 0) +
                 450 # bandwidth
  }
}

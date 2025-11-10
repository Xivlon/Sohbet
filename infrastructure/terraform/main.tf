# Main Terraform Configuration
# WebRTC Hybrid Infrastructure - Production

locals {
  common_tags = merge(
    {
      Project     = var.project_name
      Environment = var.environment
      ManagedBy   = "Terraform"
    },
    var.additional_tags
  )
}

# VPC and Networking
module "vpc" {
  source = "./modules/vpc"

  project_name       = var.project_name
  environment        = var.environment
  vpc_cidr           = var.vpc_cidr
  availability_zones = var.availability_zones
  tags               = local.common_tags
}

# Security Groups
resource "aws_security_group" "backend" {
  name        = "${var.project_name}-backend-${var.environment}"
  description = "Security group for backend instances"
  vpc_id      = module.vpc.vpc_id

  # HTTP API
  ingress {
    from_port   = 8080
    to_port     = 8080
    protocol    = "tcp"
    cidr_blocks = [var.vpc_cidr]
  }

  # WebSocket
  ingress {
    from_port   = 8081
    to_port     = 8081
    protocol    = "tcp"
    cidr_blocks = [var.vpc_cidr]
  }

  # SSH from bastion
  ingress {
    from_port       = 22
    to_port         = 22
    protocol        = "tcp"
    security_groups = [module.vpc.bastion_sg_id]
  }

  # Egress all
  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = merge(local.common_tags, {
    Name = "${var.project_name}-backend-sg"
  })
}

resource "aws_security_group" "redis" {
  name        = "${var.project_name}-redis-${var.environment}"
  description = "Security group for Redis cluster"
  vpc_id      = module.vpc.vpc_id

  # Redis port from backend
  ingress {
    from_port       = 6379
    to_port         = 6379
    protocol        = "tcp"
    security_groups = [aws_security_group.backend.id, aws_security_group.livekit.id]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = merge(local.common_tags, {
    Name = "${var.project_name}-redis-sg"
  })
}

resource "aws_security_group" "livekit" {
  name        = "${var.project_name}-livekit-${var.environment}"
  description = "Security group for LiveKit SFU"
  vpc_id      = module.vpc.vpc_id

  # WebSocket API
  ingress {
    from_port   = 7880
    to_port     = 7880
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # HTTP API
  ingress {
    from_port   = 7881
    to_port     = 7881
    protocol    = "tcp"
    cidr_blocks = [var.vpc_cidr]
  }

  # RTC ports (UDP)
  ingress {
    from_port   = 50000
    to_port     = 60000
    protocol    = "udp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # SSH
  ingress {
    from_port       = 22
    to_port         = 22
    protocol        = "tcp"
    security_groups = [module.vpc.bastion_sg_id]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = merge(local.common_tags, {
    Name = "${var.project_name}-livekit-sg"
  })
}

resource "aws_security_group" "turn" {
  name        = "${var.project_name}-turn-${var.environment}"
  description = "Security group for TURN servers"
  vpc_id      = module.vpc.vpc_id

  # TURN UDP
  ingress {
    from_port   = 3478
    to_port     = 3478
    protocol    = "udp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # TURN TCP
  ingress {
    from_port   = 3478
    to_port     = 3478
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # TURNS (TLS)
  ingress {
    from_port   = 5349
    to_port     = 5349
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # Relay ports
  ingress {
    from_port   = 49152
    to_port     = 65535
    protocol    = "udp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  # SSH
  ingress {
    from_port       = 22
    to_port         = 22
    protocol        = "tcp"
    security_groups = [module.vpc.bastion_sg_id]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = merge(local.common_tags, {
    Name = "${var.project_name}-turn-sg"
  })
}

# Redis Cluster
module "redis" {
  source = "./modules/redis"

  project_name       = var.project_name
  environment        = var.environment
  vpc_id             = module.vpc.vpc_id
  subnet_ids         = module.vpc.private_subnet_ids
  security_group_ids = [aws_security_group.redis.id]
  node_type          = var.redis_node_type
  num_cache_nodes    = var.redis_num_cache_nodes
  auth_token         = var.redis_auth_token
  tags               = local.common_tags
}

# Backend Instances
module "backend" {
  source = "./modules/backend"

  project_name       = var.project_name
  environment        = var.environment
  vpc_id             = module.vpc.vpc_id
  subnet_ids         = module.vpc.private_subnet_ids
  security_group_ids = [aws_security_group.backend.id]
  instance_count     = var.backend_instance_count
  instance_type      = var.backend_instance_type
  ami_id             = var.backend_ami_id
  redis_endpoint     = module.redis.redis_endpoint
  redis_port         = module.redis.redis_port
  redis_auth_token   = var.redis_auth_token
  tags               = local.common_tags
}

# LiveKit SFU Cluster
module "livekit" {
  source = "./modules/livekit"

  project_name       = var.project_name
  environment        = var.environment
  vpc_id             = module.vpc.vpc_id
  subnet_ids         = module.vpc.public_subnet_ids # Public for RTC
  security_group_ids = [aws_security_group.livekit.id]
  node_count         = var.livekit_node_count
  instance_type      = var.livekit_instance_type
  ami_id             = var.backend_ami_id
  redis_endpoint     = module.redis.redis_endpoint
  api_key            = var.livekit_api_key
  api_secret         = var.livekit_api_secret
  tags               = local.common_tags
}

# TURN Servers (Multi-Region)
module "turn_us" {
  source = "./modules/turn"

  project_name       = var.project_name
  environment        = var.environment
  region             = "us-east-1"
  vpc_id             = module.vpc.vpc_id
  subnet_id          = module.vpc.public_subnet_ids[0]
  security_group_ids = [aws_security_group.turn.id]
  instance_type      = var.turn_instance_type
  ami_id             = var.backend_ami_id
  turn_secret        = var.turn_secret_key
  tags               = local.common_tags
}

module "turn_eu" {
  source = "./modules/turn"
  providers = {
    aws = aws.eu
  }

  project_name       = var.project_name
  environment        = var.environment
  region             = "eu-west-1"
  vpc_id             = module.vpc.vpc_id
  subnet_id          = module.vpc.public_subnet_ids[0]
  security_group_ids = [aws_security_group.turn.id]
  instance_type      = var.turn_instance_type
  ami_id             = "ami-0d71ea30463e0ff8d" # Ubuntu 22.04 eu-west-1
  turn_secret        = var.turn_secret_key
  tags               = local.common_tags
}

module "turn_asia" {
  source = "./modules/turn"
  providers = {
    aws = aws.asia
  }

  project_name       = var.project_name
  environment        = var.environment
  region             = "ap-northeast-1"
  vpc_id             = module.vpc.vpc_id
  subnet_id          = module.vpc.public_subnet_ids[0]
  security_group_ids = [aws_security_group.turn.id]
  instance_type      = var.turn_instance_type
  ami_id             = "ami-0d52744d6551d851e" # Ubuntu 22.04 ap-northeast-1
  turn_secret        = var.turn_secret_key
  tags               = local.common_tags
}

# Application Load Balancer
module "loadbalancer" {
  source = "./modules/loadbalancer"

  project_name          = var.project_name
  environment           = var.environment
  vpc_id                = module.vpc.vpc_id
  subnet_ids            = module.vpc.public_subnet_ids
  backend_instance_ids  = module.backend.instance_ids
  certificate_arn       = var.certificate_arn
  tags                  = local.common_tags
}

# Monitoring (Prometheus + Grafana)
module "monitoring" {
  count  = var.enable_monitoring ? 1 : 0
  source = "./modules/monitoring"

  project_name       = var.project_name
  environment        = var.environment
  vpc_id             = module.vpc.vpc_id
  subnet_id          = module.vpc.public_subnet_ids[0]
  instance_type      = var.monitoring_instance_type
  ami_id             = var.backend_ami_id
  backend_ips        = module.backend.private_ips
  livekit_ips        = module.livekit.private_ips
  redis_endpoint     = module.redis.redis_endpoint
  tags               = local.common_tags
}

# Route53 DNS (Optional)
resource "aws_route53_record" "api" {
  count   = var.certificate_arn != "" ? 1 : 0
  zone_id = data.aws_route53_zone.main[0].zone_id
  name    = "api.${data.aws_route53_zone.main[0].name}"
  type    = "A"

  alias {
    name                   = module.loadbalancer.alb_dns_name
    zone_id                = module.loadbalancer.alb_zone_id
    evaluate_target_health = true
  }
}

data "aws_route53_zone" "main" {
  count = var.certificate_arn != "" ? 1 : 0
  name  = "sohbet.com" # Change to your domain
}

# WebRTC Hybrid Infrastructure - Terraform Configuration

This directory contains Infrastructure as Code (IaC) for deploying the production WebRTC hybrid system.

## Directory Structure

```
terraform/
├── main.tf                  # Main configuration
├── variables.tf             # Input variables
├── outputs.tf              # Output values
├── providers.tf            # Provider configuration
├── modules/
│   ├── vpc/                # VPC and networking
│   ├── redis/              # Redis cluster
│   ├── backend/            # Backend EC2 instances
│   ├── livekit/            # LiveKit SFU cluster
│   ├── turn/               # coturn servers
│   ├── loadbalancer/       # HAProxy/ALB
│   └── monitoring/         # Prometheus & Grafana
├── environments/
│   ├── dev/                # Development environment
│   ├── staging/            # Staging environment
│   └── production/         # Production environment
└── scripts/
    ├── install-backend.sh  # Backend installation script
    ├── install-redis.sh    # Redis setup script
    └── install-turn.sh     # TURN server setup script
```

## Prerequisites

1. **AWS Account** (or equivalent cloud provider)
2. **Terraform** >= 1.6.0
3. **AWS CLI** configured with credentials
4. **SSH key pair** for EC2 access

## Quick Start

```bash
# Initialize Terraform
cd infrastructure/terraform/environments/production
terraform init

# Plan deployment
terraform plan -out=tfplan

# Apply configuration
terraform apply tfplan

# View outputs
terraform output
```

## Estimated Costs

| Environment | Monthly Cost | Resources |
|-------------|--------------|-----------|
| **Development** | $500 | Minimal replicas, single region |
| **Staging** | $1,200 | 2 replicas, single region |
| **Production** | $2,750 | Full HA, 3 regions |

## Configuration

Copy `terraform.tfvars.example` to `terraform.tfvars` and customize:

```hcl
aws_region = "us-east-1"
environment = "production"
project_name = "sohbet-voice"

# Network
vpc_cidr = "10.0.0.0/16"

# Backend
backend_instance_count = 3
backend_instance_type = "t3.large"

# Redis
redis_node_count = 3
redis_instance_type = "cache.t3.medium"

# LiveKit
livekit_node_count = 3
livekit_instance_type = "c5.2xlarge"

# TURN
turn_regions = ["us-east-1", "eu-west-1", "ap-northeast-1"]
turn_instance_type = "t3.large"
```

## Deployment Order

Terraform will create resources in the correct order, but for manual deployments:

1. VPC and networking
2. Security groups
3. Redis cluster
4. Backend instances
5. LiveKit cluster
6. TURN servers (multi-region)
7. Load balancers
8. Monitoring stack

## Destroy Infrastructure

```bash
terraform destroy
```

⚠️ **WARNING:** This will delete all resources. Backup data first!

## Support

See `docs/WEBRTC_HYBRID_PRODUCTION_DEPLOYMENT.md` for detailed architecture.

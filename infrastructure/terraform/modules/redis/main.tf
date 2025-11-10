# Redis Cluster Module
# Creates ElastiCache Redis cluster with replication

variable "project_name" {}
variable "environment" {}
variable "vpc_id" {}
variable "subnet_ids" { type = list(string) }
variable "security_group_ids" { type = list(string) }
variable "node_type" {}
variable "num_cache_nodes" {}
variable "auth_token" { sensitive = true }
variable "tags" { type = map(string) }

resource "aws_elasticache_subnet_group" "redis" {
  name       = "${var.project_name}-redis-subnet-${var.environment}"
  subnet_ids = var.subnet_ids

  tags = merge(var.tags, {
    Name = "${var.project_name}-redis-subnet-group"
  })
}

resource "aws_elasticache_replication_group" "redis" {
  replication_group_id       = "${var.project_name}-${var.environment}"
  replication_group_description = "Redis cluster for ${var.project_name}"
  engine                     = "redis"
  engine_version             = "7.0"
  node_type                  = var.node_type
  num_cache_clusters         = var.num_cache_nodes
  port                       = 6379
  parameter_group_name       = "default.redis7"
  subnet_group_name          = aws_elasticache_subnet_group.redis.name
  security_group_ids         = var.security_group_ids

  # Authentication
  auth_token                 = var.auth_token
  transit_encryption_enabled = true
  at_rest_encryption_enabled = true

  # Automatic failover
  automatic_failover_enabled = true
  multi_az_enabled           = true

  # Maintenance
  maintenance_window = "sun:05:00-sun:06:00"
  snapshot_window    = "03:00-04:00"
  snapshot_retention_limit = 7

  # Notifications
  notification_topic_arn = aws_sns_topic.redis_alerts.arn

  tags = merge(var.tags, {
    Name = "${var.project_name}-redis-cluster"
  })
}

resource "aws_sns_topic" "redis_alerts" {
  name = "${var.project_name}-redis-alerts-${var.environment}"

  tags = var.tags
}

output "redis_endpoint" {
  value = aws_elasticache_replication_group.redis.primary_endpoint_address
}

output "redis_port" {
  value = aws_elasticache_replication_group.redis.port
}

output "redis_reader_endpoint" {
  value = aws_elasticache_replication_group.redis.reader_endpoint_address
}

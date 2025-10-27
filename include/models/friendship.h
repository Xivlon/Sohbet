#pragma once

#include <string>
#include <optional>

namespace sohbet {

class Friendship {
public:
    // Constructors
    Friendship() = default;
    Friendship(int requester_id, int addressee_id);

    // Getters
    std::optional<int> getId() const { return id_; }
    int getRequesterId() const { return requester_id_; }
    int getAddresseeId() const { return addressee_id_; }
    const std::string& getStatus() const { return status_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setRequesterId(int requester_id) { requester_id_ = requester_id; }
    void setAddresseeId(int addressee_id) { addressee_id_ = addressee_id; }
    void setStatus(const std::string& status) { status_ = status; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::optional<std::string>& updated_at) { updated_at_ = updated_at; }

    // JSON serialization
    std::string toJson() const;
    static Friendship fromJson(const std::string& json);

    // Status constants
    static constexpr const char* STATUS_PENDING = "pending";
    static constexpr const char* STATUS_ACCEPTED = "accepted";
    static constexpr const char* STATUS_REJECTED = "rejected";
    static constexpr const char* STATUS_BLOCKED = "blocked";

private:
    std::optional<int> id_;
    int requester_id_;
    int addressee_id_;
    std::string status_ = STATUS_PENDING;
    std::optional<std::string> created_at_;
    std::optional<std::string> updated_at_;
};

} // namespace sohbet

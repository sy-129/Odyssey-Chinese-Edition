import Foundation
import CryptoKit
import Security

fileprivate struct LicenseEnvelope: Codable { let payload: String; let signature: String; let refresh_token: String }

public enum LicenseError: Error {
    case invalidConfiguration, serverRejected(Int), malformedReply, invalidSignature
    case wrongInstallation, expired, wrongChallenge, keychain(OSStatus)
}
extension LicenseError: LocalizedError {
    public var errorDescription: String? {
        switch self {
        case .invalidConfiguration: return "服务器地址必须使用 HTTPS，公钥配置也必须有效。"
        case .serverRejected(let code): return "授权服务拒绝请求（HTTP \(code)），请检查授权码、设备绑定和撤销状态。"
        case .malformedReply: return "授权服务返回的数据格式无效。"
        case .invalidSignature: return "授权签名验证失败，请确认服务器与应用使用同一套签发密钥。"
        case .wrongInstallation: return "该授权不属于当前安装实例。"
        case .expired: return "授权或验证请求已过期，请重新激活。"
        case .wrongChallenge: return "授权回复与本次验证请求不匹配，请重试。"
        case .keychain(let status): return "钥匙串访问失败（系统错误 \(status)）。"
        }
    }
}

public struct SignedLicense: Decodable, Sendable {
    public let version: Int
    public let license_id: String
    public let device_id: String
    public let issued_at: Int64
    public let expires_at: Int64
    public let nonce: String
    public let scope: String
    public var isCurrent: Bool {
        let current = Int64(Date().timeIntervalSince1970)
        return version == 1 && scope == "trollspeed-rebuild" && issued_at <= current + 60 && expires_at > current
    }
}

public enum InstallationIdentity {
    public static func load() throws -> String {
        let query: [String: Any] = [kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: "local.trollspeed.rebuild.license", kSecAttrAccessGroup as String: "local.trollspeed.rebuild", kSecAttrAccount as String: "installation"]
        var item: CFTypeRef?
        var read = query
        read[kSecReturnData as String] = true
        read[kSecMatchLimit as String] = kSecMatchLimitOne
        let status = SecItemCopyMatching(read as CFDictionary, &item)
        if status == errSecSuccess {
            guard let bytes = item as? Data, let id = String(data: bytes, encoding: .utf8), !id.isEmpty else { throw LicenseError.malformedReply }
            return id
        }
        guard status == errSecItemNotFound else { throw LicenseError.keychain(status) }
        let id = UUID().uuidString
        var add = query
        add[kSecValueData as String] = Data(id.utf8)
        add[kSecAttrAccessible as String] = kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly
        let created = SecItemAdd(add as CFDictionary, nil)
        if created == errSecDuplicateItem { return try load() }
        guard created == errSecSuccess else { throw LicenseError.keychain(created) }
        return id
    }
}

public struct LicenseClient: Sendable {
    private let base: URL
    private let publicKey: Curve25519.Signing.PublicKey
    public init(baseURL: URL, publicKeyBase64: String) throws {
        guard baseURL.scheme == "https", baseURL.host != nil,
              let raw = Data(base64Encoded: publicKeyBase64) else { throw LicenseError.invalidConfiguration }
        self.base = baseURL
        self.publicKey = try Curve25519.Signing.PublicKey(rawRepresentation: raw)
    }
    private func post<Body: Encodable, Reply: Decodable>(_ path: String, _ body: Body) async throws -> Reply {
        var request = URLRequest(url: base.appendingPathComponent(path))
        request.httpMethod = "POST"
        request.timeoutInterval = 20
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request.httpBody = try JSONEncoder().encode(body)
        let (bytes, response) = try await URLSession.shared.data(for: request)
        guard let http = response as? HTTPURLResponse else { throw LicenseError.malformedReply }
        guard http.statusCode == 200 else { throw LicenseError.serverRejected(http.statusCode) }
        return try JSONDecoder().decode(Reply.self, from: bytes)
    }
    fileprivate func refresh(device: String, token: String) async throws -> SignedLicense {
        struct Body: Encodable { let device_id: String }
        struct Challenge: Decodable { let nonce: String; let expires_at: Int64 }
        struct Renewal: Encodable { let device_id: String; let refresh_token: String; let nonce: String }
        let challenge: Challenge = try await post("v1/challenge", Body(device_id: device))
        guard challenge.expires_at > Int64(Date().timeIntervalSince1970) else { throw LicenseError.expired }
        let envelope: LicenseEnvelope = try await post("v1/refresh", Renewal(device_id: device, refresh_token: token, nonce: challenge.nonce))
        guard let raw = Data(base64Encoded: envelope.payload), let signature = Data(base64Encoded: envelope.signature) else { throw LicenseError.malformedReply }
        guard publicKey.isValidSignature(signature, for: raw) else { throw LicenseError.invalidSignature }
        let license = try JSONDecoder().decode(SignedLicense.self, from: raw)
        guard license.device_id == device else { throw LicenseError.wrongInstallation }
        guard license.nonce == challenge.nonce else { throw LicenseError.wrongChallenge }
        guard license.isCurrent else { throw LicenseError.expired }
        try TSLicenseBridge.store(payload: envelope.payload, signature: envelope.signature, refreshToken: envelope.refresh_token, server: base.absoluteString)
        return license
    }
    public func activate(key: String) async throws -> SignedLicense {
        struct ChallengeBody: Encodable { let device_id: String }
        struct Challenge: Decodable { let nonce: String; let expires_at: Int64 }
        struct Activation: Encodable { let device_id: String; let license_key: String; let nonce: String }
        let installation = try InstallationIdentity.load()
        let challenge: Challenge = try await post("v1/challenge", ChallengeBody(device_id: installation))
        guard challenge.expires_at > Int64(Date().timeIntervalSince1970) else { throw LicenseError.expired }
        let envelope: LicenseEnvelope = try await post("v1/activate", Activation(device_id: installation, license_key: key, nonce: challenge.nonce))
        guard let raw = Data(base64Encoded: envelope.payload), let signature = Data(base64Encoded: envelope.signature) else { throw LicenseError.malformedReply }
        guard publicKey.isValidSignature(signature, for: raw) else { throw LicenseError.invalidSignature }
        let result = try JSONDecoder().decode(SignedLicense.self, from: raw)
        guard result.device_id == installation else { throw LicenseError.wrongInstallation }
        guard result.nonce == challenge.nonce else { throw LicenseError.wrongChallenge }
        guard result.isCurrent else { throw LicenseError.expired }
        try TSLicenseBridge.store(payload: envelope.payload, signature: envelope.signature, refreshToken: envelope.refresh_token, server: base.absoluteString)
        return result
    }
}

// The HUD independently verifies a signed lease; a UI boolean is never an authorization token.
@objc(TSLicenseBridge)
public final class TSLicenseBridge: NSObject {
    private static var leaseURL: URL {
        URL(fileURLWithPath: "/var/mobile/Library/Caches/local.trollspeed.rebuild/lease.json")
    }
    @objc public static func hasValidLease() -> Bool {
        do {
            let bytes = try Data(contentsOf: leaseURL)
            let envelope = try JSONDecoder().decode(StoredLease.self, from: bytes)
            guard let configURL = Bundle.main.url(forResource: "public-config", withExtension: "json"),
                  let config = try JSONSerialization.jsonObject(with: Data(contentsOf: configURL)) as? [String: Any],
                  let key = config["public_key_base64"] as? String,
                  let rawKey = Data(base64Encoded: key),
                  let payload = Data(base64Encoded: envelope.payload),
                  let signature = Data(base64Encoded: envelope.signature) else { return false }
            let publicKey = try Curve25519.Signing.PublicKey(rawRepresentation: rawKey)
            guard publicKey.isValidSignature(signature, for: payload) else { return false }
            let license = try JSONDecoder().decode(SignedLicense.self, from: payload)
            return license.isCurrent
        } catch { return false }
    }
    fileprivate struct StoredLease: Codable { let payload: String; let signature: String; let refresh_token: String; let server_url: String }
    fileprivate static func store(payload: String, signature: String, refreshToken: String, server: String) throws {
        let directory = leaseURL.deletingLastPathComponent()
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true,
                                               attributes: [.posixPermissions: 0o700])
        try JSONEncoder().encode(StoredLease(payload: payload, signature: signature, refresh_token: refreshToken, server_url: server)).write(to: leaseURL, options: .atomic)
        try FileManager.default.setAttributes([.posixPermissions: 0o600, .ownerAccountID: 501], ofItemAtPath: leaseURL.path)
    }
    private static let renewalQueue = DispatchQueue(label: "local.trollspeed.lease-renewal")
    private static var renewing = false
    @objc public static func renewIfNeeded() {
        let begin = renewalQueue.sync { () -> Bool in
            if renewing { return false }
            renewing = true
            return true
        }
        guard begin else { return }
        Task {
            defer { renewalQueue.sync { renewing = false } }
            do {
                let stored = try JSONDecoder().decode(StoredLease.self, from: Data(contentsOf: leaseURL))
                guard let raw = Data(base64Encoded: stored.payload) else { return }
                let lease = try JSONDecoder().decode(SignedLicense.self, from: raw)
                // Renew before expiry; the new envelope still has to pass the pinned-key signature checks.
                guard lease.expires_at - Int64(Date().timeIntervalSince1970) < 300 else { return }
                guard let url = URL(string: stored.server_url),
                      let configURL = Bundle.main.url(forResource: "public-config", withExtension: "json"),
                      let config = try JSONSerialization.jsonObject(with: Data(contentsOf: configURL)) as? [String: Any],
                      let publicKey = config["public_key_base64"] as? String else { return }
                let client = try LicenseClient(baseURL: url, publicKeyBase64: publicKey)
                _ = try await client.refresh(device: lease.device_id, token: stored.refresh_token)
            } catch { NSLog("License renewal failed: %@", String(describing: error)) }
        }
    }
    @objc public static func activate(_ server: String, key: String, completion: @escaping (Bool, String) -> Void) {
        Task {
            do {
                guard let url = URL(string: server),
                      let configURL = Bundle.main.url(forResource: "public-config", withExtension: "json"),
                      let config = try JSONSerialization.jsonObject(with: Data(contentsOf: configURL)) as? [String: Any],
                      let publicKey = config["public_key_base64"] as? String else { throw LicenseError.invalidConfiguration }
                let client = try LicenseClient(baseURL: url, publicKeyBase64: publicKey)
                let license = try await client.activate(key: key)
                await MainActor.run { completion(true, "授权有效至 \(Date(timeIntervalSince1970: TimeInterval(license.expires_at)))") }
            } catch {
                await MainActor.run { completion(false, "授权失败：\(error.localizedDescription)") }
            }
        }
    }
}

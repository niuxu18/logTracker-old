void
ntlm_make_challenge(ntlm_challenge *ch,
                    const char *domain, const char *domain_controller_UNUSED,
                    const char *challenge_nonce, const int challenge_nonce_len,
                    const uint32_t flags)
{
    int pl = 0;
    memset(ch, 0, sizeof(ntlm_challenge));	/* reset */
    memcpy(ch->hdr.signature, "NTLMSSP", 8);		/* set the signature */
    ch->hdr.type = htole32(NTLM_CHALLENGE);	/* this is a challenge */
    if (domain != NULL) {
        ntlm_add_to_payload(&ch->hdr, ch->payload, &pl, &ch->target, domain, strlen(domain));
    }
    ch->flags = htole32(flags);
    ch->context_low = 0;		/* check this out */
    ch->context_high = 0;
    memcpy(ch->challenge, challenge_nonce, challenge_nonce_len);
}
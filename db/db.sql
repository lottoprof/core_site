--
-- PostgreSQL database dump
--

-- Dumped from database version 12.11 (Ubuntu 12.11-0ubuntu0.20.04.1)
-- Dumped by pg_dump version 12.11 (Ubuntu 12.11-0ubuntu0.20.04.1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: domains; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.domains (
    id bigint NOT NULL,
    name character varying(200),
    url character varying(200),
    rank bigint DEFAULT 10
);


ALTER TABLE public.domains OWNER TO ursus;

--
-- Name: TABLE domains; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.domains IS 'Домены для рекламы';


--
-- Name: COLUMN domains.id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.domains.id IS 'Уникальный идентификатор';


--
-- Name: COLUMN domains.name; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.domains.name IS 'Произвольное имя домена';


--
-- Name: COLUMN domains.url; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.domains.url IS 'Фактический адрес домена';


--
-- Name: COLUMN domains.rank; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.domains.rank IS 'Стоимость рекламы на домене';


--
-- Name: domains_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.domains_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.domains_id_seq OWNER TO ursus;

--
-- Name: domains_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.domains_id_seq OWNED BY public.domains.id;


--
-- Name: gw; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.gw (
    id bigint NOT NULL,
    url character varying(500),
    name character varying(200)
);


ALTER TABLE public.gw OWNER TO ursus;

--
-- Name: TABLE gw; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.gw IS 'Платёжные шлюзы';


--
-- Name: gw_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.gw_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.gw_id_seq OWNER TO ursus;

--
-- Name: gw_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.gw_id_seq OWNED BY public.gw.id;


--
-- Name: messages; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.messages (
    id bigint NOT NULL,
    player_id bigint,
    body text,
    date_from timestamp without time zone,
    date_until timestamp without time zone,
    rank bigint,
    followers bigint,
    status integer,
    sign bytea,
    domain_id bigint
);


ALTER TABLE public.messages OWNER TO ursus;

--
-- Name: TABLE messages; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.messages IS 'Таблица сообщений (объявлений)';


--
-- Name: COLUMN messages.id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.id IS 'Уникальный идентификатор';


--
-- Name: COLUMN messages.player_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.player_id IS 'Ссылка на таблицу пользователей (покупателей)';


--
-- Name: COLUMN messages.body; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.body IS 'Текст сообщения';


--
-- Name: COLUMN messages.date_from; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.date_from IS 'Дата начала публикации';


--
-- Name: COLUMN messages.date_until; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.date_until IS 'Дата окончания публикации';


--
-- Name: COLUMN messages.rank; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.rank IS 'Уровень успешности';


--
-- Name: COLUMN messages.followers; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.followers IS 'Число подписчиков (фолловеров)';


--
-- Name: COLUMN messages.status; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.status IS 'Статус сообщения. 0 - на публикации. 1 - модерируется. 2 - заблокировано. 3 - удалено';


--
-- Name: COLUMN messages.sign; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.sign IS 'Подпись ПО';


--
-- Name: COLUMN messages.domain_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.messages.domain_id IS 'Ссылка на таблицу доменов';


--
-- Name: messages_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.messages_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.messages_id_seq OWNER TO ursus;

--
-- Name: messages_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.messages_id_seq OWNED BY public.messages.id;


--
-- Name: order_log; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.order_log (
    id bigint NOT NULL,
    player_id bigint,
    player_bank_id bigint,
    amount bigint,
    currency character varying(3),
    gw_id bigint,
    sign bytea,
    server_date timestamp without time zone,
    rrn character varying(200)
);


ALTER TABLE public.order_log OWNER TO ursus;

--
-- Name: TABLE order_log; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.order_log IS 'Лог пополнений';


--
-- Name: COLUMN order_log.id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.id IS 'Уникальный идентификатор';


--
-- Name: COLUMN order_log.player_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.player_id IS 'Ссылка на таблицу покупателей';


--
-- Name: COLUMN order_log.player_bank_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.player_bank_id IS 'Ссылка на таблицу баланса (токенов)';


--
-- Name: COLUMN order_log.amount; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.amount IS 'Сумма пополнения';


--
-- Name: COLUMN order_log.currency; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.currency IS 'Валюта';


--
-- Name: COLUMN order_log.gw_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.gw_id IS 'Ссылка на шлюз';


--
-- Name: COLUMN order_log.sign; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.sign IS 'Подпись ПО';


--
-- Name: COLUMN order_log.server_date; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.server_date IS 'Дата исполнения транзакции';


--
-- Name: COLUMN order_log.rrn; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.order_log.rrn IS 'Идентификатор пластиковой транзакции';


--
-- Name: order_log_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.order_log_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.order_log_id_seq OWNER TO ursus;

--
-- Name: order_log_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.order_log_id_seq OWNED BY public.order_log.id;


--
-- Name: payout_log; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.payout_log (
    id bigint NOT NULL,
    player_id bigint,
    player_bank_id bigint,
    amount bigint,
    currency character varying(3),
    gw_id bigint,
    server_date timestamp without time zone,
    rrn character varying(200),
    sign bytea
);


ALTER TABLE public.payout_log OWNER TO ursus;

--
-- Name: TABLE payout_log; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.payout_log IS 'Лог вывода средств';


--
-- Name: COLUMN payout_log.id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.id IS 'Уникальный идентификатор';


--
-- Name: COLUMN payout_log.player_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.player_id IS 'Ссылка на таблицу покупателей';


--
-- Name: COLUMN payout_log.player_bank_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.player_bank_id IS 'Ссылка на таблицу баланса (токенов)';


--
-- Name: COLUMN payout_log.amount; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.amount IS 'Сумма вывода';


--
-- Name: COLUMN payout_log.currency; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.currency IS 'Валюта';


--
-- Name: COLUMN payout_log.gw_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.gw_id IS 'Ссылка на шлюз';


--
-- Name: COLUMN payout_log.server_date; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.server_date IS 'Дата исполнения транзакции';


--
-- Name: COLUMN payout_log.rrn; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.rrn IS 'Идентификатор пластиковой транзакции';


--
-- Name: COLUMN payout_log.sign; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.payout_log.sign IS 'Подпись ПО';


--
-- Name: payout_log_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.payout_log_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.payout_log_id_seq OWNER TO ursus;

--
-- Name: payout_log_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.payout_log_id_seq OWNED BY public.payout_log.id;


--
-- Name: player; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.player (
    id bigint NOT NULL,
    phone bigint,
    token bigint,
    date_from timestamp without time zone,
    date_until timestamp without time zone,
    status bigint,
    last_connection timestamp without time zone,
    sign bytea
);


ALTER TABLE public.player OWNER TO ursus;

--
-- Name: TABLE player; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.player IS 'Таблица пользователей';


--
-- Name: COLUMN player.id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.id IS 'Уникальный идентификатор пользователя.';


--
-- Name: COLUMN player.phone; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.phone IS 'Номер телефона';


--
-- Name: COLUMN player.token; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.token IS 'Токен авторизации';


--
-- Name: COLUMN player.date_from; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.date_from IS 'Дата регистрации';


--
-- Name: COLUMN player.date_until; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.date_until IS 'Дата окончания действия пользователя';


--
-- Name: COLUMN player.status; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.status IS 'Статус. 0 - активен. 1 - заблокирован ввод средств. 2 - заблокирован вывод средств. 3 - мошеннические действия';


--
-- Name: COLUMN player.last_connection; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.last_connection IS 'Дата последнего подключения';


--
-- Name: COLUMN player.sign; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player.sign IS 'Подпись ПО';


--
-- Name: player_bank; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.player_bank (
    id bigint NOT NULL,
    player_id bigint,
    amount bigint,
    currency character(3),
    date_from timestamp without time zone,
    date_until timestamp without time zone,
    cert bytea,
    sign bytea,
    token bytea,
    status integer
);


ALTER TABLE public.player_bank OWNER TO ursus;

--
-- Name: TABLE player_bank; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.player_bank IS 'Таблица денег (токенов) пользователей';


--
-- Name: COLUMN player_bank.id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.id IS 'Уникальный идентификатор записи';


--
-- Name: COLUMN player_bank.player_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.player_id IS 'Ссылка на таблицу пользователей';


--
-- Name: COLUMN player_bank.amount; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.amount IS 'Сумма в центах';


--
-- Name: COLUMN player_bank.currency; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.currency IS 'Валюта. RUB, USD, SOM';


--
-- Name: COLUMN player_bank.date_from; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.date_from IS 'Дата начала действия выплаты';


--
-- Name: COLUMN player_bank.date_until; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.date_until IS 'Дата окончания действия выплаты';


--
-- Name: COLUMN player_bank.cert; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.cert IS 'Сертификат токена';


--
-- Name: COLUMN player_bank.sign; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.sign IS 'Подпись ПО';


--
-- Name: COLUMN player_bank.token; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.token IS 'Сам токен (либо криптоконтейнер)';


--
-- Name: COLUMN player_bank.status; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_bank.status IS 'Статус токена. 0 - Доступен. 1 - Разбит на слоты. 2 - Пополняется. 3 - Списывается. 6 - заблокирован';


--
-- Name: player_bank_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.player_bank_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.player_bank_id_seq OWNER TO ursus;

--
-- Name: player_bank_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.player_bank_id_seq OWNED BY public.player_bank.id;


--
-- Name: player_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.player_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.player_id_seq OWNER TO ursus;

--
-- Name: player_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.player_id_seq OWNED BY public.player.id;


--
-- Name: player_keys; Type: TABLE; Schema: public; Owner: ursus
--

CREATE TABLE public.player_keys (
    id bigint NOT NULL,
    player_id bigint NOT NULL,
    public_key bytea,
    cert bytea,
    sign bytea
);


ALTER TABLE public.player_keys OWNER TO ursus;

--
-- Name: TABLE player_keys; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON TABLE public.player_keys IS 'Таблица ключей пользователя';


--
-- Name: COLUMN player_keys.id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_keys.id IS 'Уникальный идентификатор ключа';


--
-- Name: COLUMN player_keys.player_id; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_keys.player_id IS 'Ссылка на таблицу игроков';


--
-- Name: COLUMN player_keys.public_key; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_keys.public_key IS 'Публичный ключ пользователя';


--
-- Name: COLUMN player_keys.cert; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_keys.cert IS 'Сертификат x509';


--
-- Name: COLUMN player_keys.sign; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON COLUMN public.player_keys.sign IS 'Подпись ПО';


--
-- Name: player_keys_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.player_keys_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.player_keys_id_seq OWNER TO ursus;

--
-- Name: player_keys_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.player_keys_id_seq OWNED BY public.player_keys.id;


--
-- Name: player_keys_player_id_seq; Type: SEQUENCE; Schema: public; Owner: ursus
--

CREATE SEQUENCE public.player_keys_player_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.player_keys_player_id_seq OWNER TO ursus;

--
-- Name: player_keys_player_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: ursus
--

ALTER SEQUENCE public.player_keys_player_id_seq OWNED BY public.player_keys.player_id;


--
-- Name: domains id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.domains ALTER COLUMN id SET DEFAULT nextval('public.domains_id_seq'::regclass);


--
-- Name: gw id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.gw ALTER COLUMN id SET DEFAULT nextval('public.gw_id_seq'::regclass);


--
-- Name: messages id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.messages ALTER COLUMN id SET DEFAULT nextval('public.messages_id_seq'::regclass);


--
-- Name: order_log id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.order_log ALTER COLUMN id SET DEFAULT nextval('public.order_log_id_seq'::regclass);


--
-- Name: payout_log id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.payout_log ALTER COLUMN id SET DEFAULT nextval('public.payout_log_id_seq'::regclass);


--
-- Name: player id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player ALTER COLUMN id SET DEFAULT nextval('public.player_id_seq'::regclass);


--
-- Name: player_bank id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player_bank ALTER COLUMN id SET DEFAULT nextval('public.player_bank_id_seq'::regclass);


--
-- Name: player_keys id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player_keys ALTER COLUMN id SET DEFAULT nextval('public.player_keys_id_seq'::regclass);


--
-- Name: player_keys player_id; Type: DEFAULT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player_keys ALTER COLUMN player_id SET DEFAULT nextval('public.player_keys_player_id_seq'::regclass);


--
-- Data for Name: domains; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.domains (id, name, url, rank) FROM stdin;
\.


--
-- Data for Name: gw; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.gw (id, url, name) FROM stdin;
\.


--
-- Data for Name: messages; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.messages (id, player_id, body, date_from, date_until, rank, followers, status, sign, domain_id) FROM stdin;
\.


--
-- Data for Name: order_log; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.order_log (id, player_id, player_bank_id, amount, currency, gw_id, sign, server_date, rrn) FROM stdin;
\.


--
-- Data for Name: payout_log; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.payout_log (id, player_id, player_bank_id, amount, currency, gw_id, server_date, rrn, sign) FROM stdin;
\.


--
-- Data for Name: player; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.player (id, phone, token, date_from, date_until, status, last_connection, sign) FROM stdin;
\.


--
-- Data for Name: player_bank; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.player_bank (id, player_id, amount, currency, date_from, date_until, cert, sign, token, status) FROM stdin;
\.


--
-- Data for Name: player_keys; Type: TABLE DATA; Schema: public; Owner: ursus
--

COPY public.player_keys (id, player_id, public_key, cert, sign) FROM stdin;
\.


--
-- Name: domains_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.domains_id_seq', 1, false);


--
-- Name: gw_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.gw_id_seq', 1, false);


--
-- Name: messages_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.messages_id_seq', 1, false);


--
-- Name: order_log_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.order_log_id_seq', 1, false);


--
-- Name: payout_log_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.payout_log_id_seq', 1, false);


--
-- Name: player_bank_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.player_bank_id_seq', 1, false);


--
-- Name: player_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.player_id_seq', 1, false);


--
-- Name: player_keys_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.player_keys_id_seq', 1, false);


--
-- Name: player_keys_player_id_seq; Type: SEQUENCE SET; Schema: public; Owner: ursus
--

SELECT pg_catalog.setval('public.player_keys_player_id_seq', 1, false);


--
-- Name: domains domains_id_uniq; Type: CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.domains
    ADD CONSTRAINT domains_id_uniq UNIQUE (id);


--
-- Name: CONSTRAINT domains_id_uniq ON domains; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON CONSTRAINT domains_id_uniq ON public.domains IS 'Уникальность идентификатора домена';


--
-- Name: player_bank player_bank_id_unique; Type: CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player_bank
    ADD CONSTRAINT player_bank_id_unique UNIQUE (id);


--
-- Name: player player_id_uniq; Type: CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player
    ADD CONSTRAINT player_id_uniq UNIQUE (id);


--
-- Name: CONSTRAINT player_id_uniq ON player; Type: COMMENT; Schema: public; Owner: ursus
--

COMMENT ON CONSTRAINT player_id_uniq ON public.player IS 'Уникальность идентификатора пользователя';


--
-- Name: gw qw_id_unique; Type: CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.gw
    ADD CONSTRAINT qw_id_unique UNIQUE (id);


--
-- Name: messages messages_domain_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_domain_id_fkey FOREIGN KEY (domain_id) REFERENCES public.domains(id);


--
-- Name: messages messages_player_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_player_id_fkey FOREIGN KEY (player_id) REFERENCES public.player(id);


--
-- Name: order_log order_log_gw_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.order_log
    ADD CONSTRAINT order_log_gw_id_fkey FOREIGN KEY (gw_id) REFERENCES public.gw(id);


--
-- Name: order_log order_log_player_bank_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.order_log
    ADD CONSTRAINT order_log_player_bank_id_fkey FOREIGN KEY (player_bank_id) REFERENCES public.player_bank(id);


--
-- Name: order_log order_log_player_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.order_log
    ADD CONSTRAINT order_log_player_id_fkey FOREIGN KEY (player_id) REFERENCES public.player(id);


--
-- Name: payout_log payout_log_gw_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.payout_log
    ADD CONSTRAINT payout_log_gw_id_fkey FOREIGN KEY (gw_id) REFERENCES public.gw(id);


--
-- Name: payout_log payout_log_player_bank_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.payout_log
    ADD CONSTRAINT payout_log_player_bank_id_fkey FOREIGN KEY (player_bank_id) REFERENCES public.player_bank(id);


--
-- Name: payout_log payout_log_player_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.payout_log
    ADD CONSTRAINT payout_log_player_id_fkey FOREIGN KEY (player_id) REFERENCES public.player(id);


--
-- Name: player_bank player_bank_player_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player_bank
    ADD CONSTRAINT player_bank_player_id_fkey FOREIGN KEY (player_id) REFERENCES public.player(id);


--
-- Name: player_keys player_keys_player_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: ursus
--

ALTER TABLE ONLY public.player_keys
    ADD CONSTRAINT player_keys_player_id_fkey FOREIGN KEY (player_id) REFERENCES public.player(id);


--
-- PostgreSQL database dump complete
--

